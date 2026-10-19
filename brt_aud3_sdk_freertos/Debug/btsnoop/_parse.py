import struct, os, sys, datetime

def u16(b, o=0): return struct.unpack_from('<H', b, o)[0]
def u32(b, o=0): return struct.unpack_from('<I', b, o)[0]

def load(path):
    data = open(path, 'rb').read()
    assert data[:8] == b'btsnoop\x00'
    ver, datalink = struct.unpack_from('>II', data, 8)
    off, recs, idx = 16, [], 0
    while off + 24 <= len(data):
        orig, incl, flags, drops = struct.unpack_from('>IIII', data, off)
        ts = struct.unpack_from('>q', data, off + 16)[0]
        pkt = data[off + 24: off + 24 + incl]
        off += 24 + incl
        idx += 1
        recs.append({'n': idx, 'flags': flags, 'ts': ts, 'pkt': pkt})
    return recs

PSM_NAME = {0x0001: 'SDP', 0x0003: 'RFCOMM', 0x0007: 'TCS-BIN', 0x000F: 'BNEP',
            0x0011: 'HID-Control', 0x0013: 'HID-Interrupt', 0x0017: 'AVCTP-AVRCP',
            0x0019: 'AVDTP-A2DP', 0x001B: 'AVCTP-Browsing', 0x0100: 'AT-Emu(HFP)',
            0x0103: 'OBEX/PBAP', 0x0002: '?', 0x0015: '?', 0x0023: 'AVDTP?', 0x0105: '?',
            0x0107: 'MAP', 0x0109: '?', 0x0005: '?', 0x0111: '?', 0x001A: '?', 0x001D: '?'}
AVC_OP = {0x40: 'Power', 0x41: 'VOL-UP', 0x42: 'VOL-DOWN', 0x43: 'MUTE', 0x44: 'PLAY',
          0x45: 'STOP', 0x46: 'PAUSE', 0x47: 'RECORD', 0x48: 'REWIND', 0x49: 'FAST-FWD',
          0x4A: 'EJECT', 0x4B: 'NEXT', 0x4C: 'PREV', 0x50: 'FWD', 0x51: 'BACK',
          0x60: 'CH-UP', 0x61: 'CH-DOWN', 0x63: 'MENU', 0x70: 'OK', 0x74: 'UP', 0x75: 'DOWN'}
AVRCP_PDU = {0x10: 'GetCapabilities', 0x11: 'ListPlayerAppSettingAttr', 0x12: 'GetPlayerAppSettingValue',
             0x13: 'SetPlayerAppSettingValue', 0x14: 'GetPlayerAppSettingAttrText',
             0x15: 'GetPlayerAppSettingValueText', 0x16: 'Information', 0x17: 'GetElementAttributes',
             0x18: 'GetElementAttributesText', 0x20: 'RegisterNotification', 0x30: 'SetAbsoluteVolume',
             0x31: 'SetAddressedPlayer', 0x40: 'SetBrowsedPlayer', 0x50: 'Search', 0x60: 'GetFolderItems',
             0x70: 'GetTotalNumberOfItems', 0x71: 'ChangePath', 0x72: 'GetItemAttributes',
             0x73: 'PlayItem', 0x74: 'GetNowPlaying'}
HIDP_TT = {0x0: 'HANDSHAKE', 0x1: 'HID_CONTROL', 0x4: 'GET_REPORT', 0x5: 'SET_REPORT',
           0x6: 'GET_PROTOCOL', 0x7: 'SET_PROTOCOL', 0x8: 'GET_IDLE', 0x9: 'SET_IDLE', 0xA: 'DATA'}
HAND = {0x00: 'SUCCESS', 0x01: 'NOT_READY', 0x02: 'ERR_INVALID_REPORT_ID',
        0x03: 'ERR_UNSUPPORTED_REQ', 0x04: 'ERR_INVALID_PARAMETER', 0x0E: 'ERR_UNKNOWN', 0x0F: 'ERR_FATAL'}
HIDCTL = {0x10: 'SUSPEND', 0x11: 'EXIT_SUSPEND', 0x12: 'VIRTUAL_CABLE_UNPLUG'}

def hexs(b):
    return ' '.join('%02X' % x for x in b)

def analyze(path):
    recs = load(path)
    t0 = recs[0]['ts']
    def ts(r): return (r['ts'] - t0) / 1e6
    # ---- reassemble L2CAP per ACL handle ----
    bufs, cid_psm, events, lines = {}, {}, [], []
    smsg = {}
    for r in recs:
        p = r['pkt']
        if not p: continue
        if p[0] == 0x04:  # HCI Event
            ev, ln = p[1], p[2]
            d = p[3:3 + ln]
            events.append((r['n'], ts(r), ev, d, r))
        elif p[0] == 0x02:  # HCI ACL
            hd = u16(p, 1)
            h = hd & 0x0FFF
            pb = (hd >> 12) & 3
            bc = (hd >> 14) & 3
            dlen = u16(p, 3)
            payload = p[5:5 + dlen]
            sent = (r['flags'] & 0x01) == 0
            key = h
            b = bufs.setdefault(key, bytearray())
            if pb != 0x00:  # start/complete
                del b[:]
            b += payload
            while len(b) >= 4:
                l = u16(b, 0)
                cid = u16(b, 2)
                if len(b) < 4 + l: break
                pdu = bytes(b[4:4 + l])
                del b[:4 + l]
                dec = decode_l2cap(cid, pdu, cid_psm, sent, ts(r), r['n'])
                if dec: lines.append((ts(r), r['n'], sent, cid, dec))
    return recs, lines, cid_psm, events, ts

def decode_l2cap(cid, pdu, cid_psm, sent, t, n):
    if cid == 0x0001:
        code, ident, ln = pdu[0], pdu[1], u16(pdu, 2)
        d = pdu[4:4 + ln]
        if code == 0x02 and len(d) >= 4:
            psm, scid = u16(d, 0), u16(d, 2)
            cid_psm[scid] = PSM_NAME.get(psm, 'PSM-0x%04X' % psm)
            return 'SIG CONN_REQ psm=0x%04X(%s) scid=0x%04X' % (psm, PSM_NAME.get(psm, '?'), scid)
        if code == 0x03 and len(d) >= 8:
            dcid, scid, res, st = u16(d, 0), u16(d, 2), u16(d, 4), u16(d, 6)
            if res == 0:
                p = cid_psm.get(scid) or cid_psm.get(dcid) or 'unknown'
                cid_psm[dcid] = p
                cid_psm[scid] = p
            return 'SIG CONN_RSP cidA=0x%04X cidB=0x%04X result=%d status=%d' % (dcid, scid, res, st)
        if code == 0x04:
            return 'SIG CONFIG_REQ dcid=0x%04X %s' % (u16(d, 0), hexs(d[4:]))
        if code == 0x05:
            return 'SIG CONFIG_RSP scid=0x%04X %s' % (u16(d, 0), hexs(d[4:]))
        if code == 0x06:
            return 'SIG DISCONN_REQ dcid=0x%04X scid=0x%04X' % (u16(d, 0), u16(d, 2))
        if code == 0x07:
            return 'SIG DISCONN_RSP dcid=0x%04X scid=0x%04X' % (u16(d, 0), u16(d, 2))
        if code == 0x08:
            return 'SIG ECHO_REQ'
        if code == 0x0A and len(d) >= 2:
            return 'SIG INFO_REQ type=0x%04X' % u16(d, 0)
        if code == 0x0B:
            return 'SIG INFO_RSP %s' % hexs(d[:8])
        return 'SIG code=0x%02X id=%d %s' % (code, ident, hexs(d[:12]))
    psm = cid_psm.get(cid)
    if psm == 'HID-Control' or psm == 'HID-Interrupt':
        hdr = pdu[0]
        tt = (hdr >> 4) & 0xF
        par = hdr & 0x0F
        nm = HIDP_TT.get(tt, 'TT%X' % tt)
        rest = pdu[1:]
        extra = ''
        if tt in (0x4, 0x5, 0x6, 0x7, 0x8, 0x9) and rest:
            pl = rest[0]
            extra = ' plen=%d %s' % (pl, hexs(rest[1:1 + pl]))
            if tt in (0x4, 0x5) and pl >= 2:
                extra += ' [rtype=%d rid=%d]' % (rest[1], rest[2])
        if tt == 0x0:
            extra = ' result=%s' % HAND.get(par, par)
        if tt == 0x1:
            extra = ' %s' % HIDCTL.get(hdr, '')
        if tt == 0xA:
            extra = ' report=%s val=%s' % (hexs(rest[:1]), hexs(rest[1:]))
        return 'HIDP %s(%s)%s' % (nm, 'CTRL' if psm == 'HID-Control' else 'INTR', extra)
    if psm in ('AVCTP-AVRCP', 'AVCTP-Browsing'):
        if len(pdu) < 6:
            return 'AVCTP %s' % hexs(pdu)
        tl = pdu[0] >> 4
        pt = (pdu[0] >> 2) & 3
        cr = (pdu[0] >> 1) & 1
        pid = (pdu[1] << 8) | pdu[2]
        CT = {0: 'CTRL', 1: 'STATUS', 2: 'SPEC_INQ', 3: 'NOTIFY', 4: 'GEN_INQ', 8: 'NOT_IMPL',
              9: 'ACCEPTED', 0xA: 'REJECTED', 0xB: 'IN_TRANS', 0xC: 'STABLE', 0xD: 'CHANGED', 0xF: 'INTERIM'}
        EV = {1: 'PLAY_STATUS_CHANGED', 2: 'TRACK_CHANGED', 3: 'TRACK_END', 4: 'TRACK_START',
              5: 'PLAY_POS_CHANGED', 6: 'BATT_STATUS', 7: 'SYS_STATUS', 8: 'APP_SETTING_CHANGED',
              9: 'NOW_PLAYING_CONTENT', 0xA: 'AVAIL_PLAYERS', 0xB: 'ADDR_PLAYER', 0xC: 'UIDS_CHANGED',
              0xD: 'VOLUME_CHANGED'}
        PD = {0x10: 'GetCapabilities', 0x20: 'GetElementAttributes', 0x31: 'RegisterNotification',
              0x50: 'SetAbsoluteVolume', 0x40: 'SetAddressedPlayer', 0x70: 'GetTotalNumItems'}
        if pid != 0x110E:
            return 'AVCTP pid=0x%04X %s' % (pid, hexs(pdu))
        ctype = pdu[3] & 0x0F
        op = pdu[5]
        if op == 0x00 and len(pdu) >= 13:
            apdu = pdu[9]
            plen = (pdu[11] << 8) | pdu[12]
            params = pdu[13:13 + plen]
            if apdu == 0x31:
                ev = params[0] if params else -1
                extra = 'RegNotify ev=0x%02X(%s) %s' % (ev, EV.get(ev, '?'), hexs(params))
            elif apdu == 0x10:
                extra = 'GetCapabilities %s' % hexs(params)
            elif apdu == 0x20:
                extra = 'GetElementAttr %s' % hexs(params[:9])
            else:
                extra = 'PDU0x%02X(%s) %s' % (apdu, PD.get(apdu, '?'), hexs(params[:12]))
            return 'AVRCP%s tl=%d cr=%d ct=%-8s %s || %s' % ('B' if 'Browsing' in psm else '', tl, cr, CT.get(ctype, '?'), extra, hexs(pdu))
        if op == 0x7C and len(pdu) >= 8:
            oid = pdu[6]
            return 'AVRCP PASS-THROUGH ct=%s op=%s state=0x%02X' % (CT.get(ctype, '?'), AVC_OP.get(oid, '0x%02X' % oid), pdu[7])
        return 'AVRCP ct=%s op=0x%02X %s' % (CT.get(ctype, '?'), op, hexs(pdu[:16]))
    if psm == 'AVDTP-A2DP':
        if len(pdu) >= 2:
            return 'AVDTP sig=%d %s' % (pdu[1] & 3, hexs(pdu[:12]))
        return 'AVDTP %s' % hexs(pdu)
    return '%s [cid=0x%04X] %s' % (psm or '?', cid, hexs(pdu[:20]))

if __name__ == '__main__':
    mode = sys.argv[1]
    for path in sys.argv[2:]:
        recs, lines, cid_psm, events, ts = analyze(path)
        print('====', os.path.basename(path), 'records=%d' % len(recs))
        print('   seconds span: %.1f' % ts(recs[-1]))
        print('   CID->PSM:', {hex(k): v for k, v in cid_psm.items()})
        print('   --- L2CAP/HIDP/AVCTP frames (%d) ---' % len(lines))
        for t, n, sent, cid, dec in lines:
            if mode == 'all' or mode in dec:
                print('   %8.3f #%-5d %s cid=0x%04X %s' % (t, n, 'H>D' if sent else 'D>H', cid, dec))
            elif mode == 'raw' and ('AVR' in dec or 'AVC' in dec or 'HIDP' in dec):
                print('   %8.3f #%-5d %s cid=0x%04X %s' % (t, n, 'H>D' if sent else 'D>H', cid, dec))
        if mode not in ('all', 'evt'):
            if mode == 'corr':
                hid = [(t, n) for t, n, s, c, d in lines if 'HIDP DATA(INTR)' in d and 'val=02' in d]
                tc = [(t, n, d) for t, n, s, c, d in lines if 'TRACK_CHANGED' in d and ('ct=CHANGED' in d or 'ct=INTERIM' in d)]
                ps = [(t, n, d) for t, n, s, c, d in lines if 'PLAY_STATUS_CHANGED' in d and ('ct=CHANGED' in d or 'ct=INTERIM' in d)]
                pt = [(t, n, d) for t, n, s, c, d in lines if 'PASS-THROUGH' in d]
                print('   >>> HID "next-track" presses: %d' % len(hid))
                for t, n in hid:
                    nxt = [x for x in tc if 0 <= x[0] - t <= 5]
                    print('      %8.3f #%-5d -> TRACK_CHANGED within 5s: %s' % (
                        t, n, ['@%.3f #%d' % (x[0], x[1]) for x in nxt] or '*** NONE ***'))
                print('   >>> TRACK_CHANGED notify (INTERIM/CHANGED): %d' % len(tc))
                for t, n, d in tc:
                    print('      %8.3f #%-5d %s' % (t, n, d))
                print('   >>> PLAY_STATUS_CHANGED notify (INTERIM/CHANGED): %d' % len(ps))
                for t, n, d in ps:
                    print('      %8.3f #%-5d %s' % (t, n, d))
                print('   >>> PASS-THROUGH (AVRCP key) count: %d' % len(pt))
                for t, n, d in pt[:20]:
                    print('      %8.3f #%-5d %s' % (t, n, d))
            continue
        print('   --- HCI events (%d) ---' % len(events))
        for n, t, ev, d, r in events:
            if ev == 0x03 and len(d) >= 3:
                print('   %8.3f #%-5d ConnectionComplete status=%d handle=0x%04X %s' % (
                    t, n, d[0], u16(d, 1) & 0xFFF, ':'.join('%02X' % x for x in reversed(d[3:9]))))
            elif ev == 0x05 and len(d) >= 4:
                print('   %8.3f #%-5d DisconnectionComplete status=%d handle=0x%04X reason=0x%02X' % (
                    t, n, d[0], u16(d, 1) & 0xFFF, d[3]))
            elif ev == 0x0E and len(d) >= 4:
                print('   %8.3f #%-5d CommandComplete opcode=0x%04X %s' % (t, n, u16(d, 3), hexs(d[4:12])))
            elif ev == 0x0F and len(d) >= 4:
                print('   %8.3f #%-5d CommandStatus opcode=0x%04X status=%d' % (t, n, u16(d, 3), d[0]))

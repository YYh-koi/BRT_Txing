# -*- coding: utf-8 -*-
"""
生成「每次 HID 按键前后 +/-5s 关键帧对照表」
用法: python _ctx.py <btsnoop1> [btsnoop2]  -> 输出 key_ctx.txt

方向约定: 本份为手机侧 HCI 抓包
    TX = 抓包端(手机)发出   -> 手机回报的 AVRCP 元数据 / 播放状态 / 通知
    RX = 仪表发来           -> HID 按键报告(仪表=HID Device)
"""
import struct, sys, os

def u16(b, o=0): return struct.unpack_from('<H', b, o)[0]
def u32(b, o=0): return struct.unpack_from('<I', b, o)[0]

def load(path):
    data = open(path, 'rb').read()
    assert data[:8] == b'btsnoop\x00'
    off, recs, idx = 16, [], 0
    while off + 24 <= len(data):
        orig, incl, flags, drops = struct.unpack_from('>IIII', data, off)
        ts = struct.unpack_from('>q', data, off + 16)[0]
        pkt = data[off + 24: off + 24 + incl]
        off += 24 + incl
        idx += 1
        recs.append({'n': idx, 'flags': flags, 'ts': ts, 'pkt': pkt})
    return recs

HID_KEY = {0x01: 'PLAY/PAUSE', 0x02: 'NEXT', 0x04: 'PREV',
           0x08: 'VOL+', 0x10: 'VOL-', 0x20: 'MUTE', 0x40: 'STOP'}
PLAYSTAT = {0: 'STOPPED', 1: 'PLAYING', 2: 'PAUSED', 3: 'FWD', 4: 'REWIND'}
EV = {1: 'PLAY_STATUS_CHANGED', 2: 'TRACK_CHANGED', 3: 'TRACK_END', 4: 'TRACK_START',
      5: 'PLAY_POS_CHANGED', 6: 'BATT_STATUS', 7: 'SYS_STATUS', 8: 'APP_SETTING_CHANGED',
      9: 'NOW_PLAYING_CONTENT', 0xA: 'AVAIL_PLAYERS', 0xB: 'ADDR_PLAYER',
      0xC: 'UIDS_CHANGED', 0xD: 'VOLUME_CHANGED'}
CTYPE = {0: 'CTRL', 1: 'STATUS', 2: 'SPEC_INQ', 3: 'NOTIFY', 4: 'GEN_INQ', 8: 'NOT_IMPL',
         9: 'ACCEPTED', 0xA: 'REJECTED', 0xB: 'IN_TRANS', 0xC: 'STABLE',
         0xD: 'CHANGED', 0xF: 'INTERIM'}
AVC_OP = {0x41: 'VOL+', 0x42: 'VOL-', 0x43: 'MUTE', 0x44: 'PLAY', 0x45: 'STOP',
          0x46: 'PAUSE', 0x4B: 'NEXT', 0x4C: 'PREV'}
HAND = {0x00: 'SUCCESS', 0x01: 'NOT_READY', 0x02: 'ERR_INVALID_REPORT_ID',
        0x03: 'ERR_UNSUPPORTED_REQ', 0x04: 'ERR_INVALID_PARAMETER',
        0x0E: 'ERR_UNKNOWN', 0x0F: 'ERR_FATAL'}
HIDCTL = {0x10: 'SUSPEND', 0x11: 'EXIT_SUSPEND', 0x12: 'VIRTUAL_CABLE_UNPLUG'}

TAGS = [(b'\x00\x00\x00\x01\x00\x6a', 'title'), (b'\x00\x00\x00\x02\x00\x6a', 'artist'),
        (b'\x00\x00\x00\x03\x00\x6a', 'album'), (b'\x00\x00\x00\x07\x00\x6a', 'length')]

def attrs(pdu):
    """从 GetElementAttributes 响应里解出 Title/Artist/Album/PlayingTime"""
    res = {}
    for tag, key in TAGS:
        i = pdu.find(tag)
        if i < 0:
            continue
        ln = (pdu[i + 6] << 8) | pdu[i + 7]
        if ln > 400 or i + 8 + ln > len(pdu):
            continue
        res[key] = pdu[i + 8:i + 8 + ln].decode('utf-8', 'replace')
    return res

def decode(cid, pdu, cid_psm, n, t, tx):
    if cid == 0x0001:  # L2CAP 信令, 只用来建 cid->psm 映射
        if len(pdu) < 4:
            return None
        code = pdu[0]
        ln = u16(pdu, 2)
        d = pdu[4:4 + ln]
        if code == 0x02 and len(d) >= 4:
            cid_psm[u16(d, 2)] = u16(d, 0)
        elif code == 0x03 and len(d) >= 8:
            if u16(d, 4) == 0:
                psm = cid_psm.get(u16(d, 2)) or cid_psm.get(u16(d, 0))
                cid_psm[u16(d, 0)] = psm
                cid_psm[u16(d, 2)] = psm
        return None

    psm = cid_psm.get(cid)

    if psm in (0x0011, 0x0013):          # HID Control / Interrupt
        hdr = pdu[0]
        tt = (hdr >> 4) & 0xF
        par = hdr & 0xF
        if tt == 0x0:
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'HIDERR',
                    'text': 'HIDP HANDSHAKE result=%s' % HAND.get(par, hex(par))}
        if tt == 0x1:
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'HIDCTL',
                    'text': 'HIDP HID_CONTROL %s' % HIDCTL.get(hdr, hex(hdr))}
        if tt in (0x7, 0x9):
            plc = pdu[2] if len(pdu) > 2 else -1
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'HIDSET',
                    'text': 'HIDP %s protocol/idle=%s' % ('SET_PROTOCOL' if tt == 0x7 else 'SET_IDLE', hex(plc))}
        if tt == 0xA:
            rep = pdu[1] if len(pdu) > 1 else -1
            val = pdu[2] if len(pdu) > 2 else 0
            rtype = {0: 'Other', 1: 'Input', 2: 'Output', 3: 'Feature'}.get(par, '?')
            if rtype != 'Input':
                return None
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'HID', 'val': val, 'rep': rep,
                    'raw': ' '.join('%02X' % x for x in pdu),
                    'text': 'HIDP DATA Input report=%d val=0x%02X (%s)' % (
                        rep, val, HID_KEY.get(val, 'release' if val == 0 else '-'))}
        return None

    if psm in (0x0017, 0x001B):          # AVCTP / AVRCP
        if len(pdu) < 6:
            return None
        ctype = pdu[3] & 0x0F
        op = pdu[5]
        if op == 0x7C and len(pdu) >= 8:   # PASS-THROUGH
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'TRANSPORT',
                    'text': 'AVRCP PASS-THROUGH ct=%s op=%s state=0x%02X' % (
                        CTYPE.get(ctype, '?'), AVC_OP.get(pdu[6], hex(pdu[6])), pdu[7])}
        if op != 0x00 or len(pdu) < 13:
            return None
        apdu = pdu[9]
        plen = (pdu[11] << 8) | pdu[12]
        params = pdu[13:13 + plen]
        if apdu == 0x20:                   # GetElementAttributes
            a = attrs(pdu)
            if not a:
                return None
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'META', 'meta': a}
        if apdu == 0x30 and len(params) >= 9:   # GetPlayStatus
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'PLAYSTATUS',
                    'len': u32(params, 0), 'pos': u32(params, 4),
                    'status': PLAYSTAT.get(params[8], params[8])}
        if apdu == 0x31 and params:        # RegisterNotification
            ev = params[0]
            return {'t': t, 'n': n, 'tx': tx, 'kind': 'NOTIFY', 'ev': ev,
                    'ctype': CTYPE.get(ctype, '?'), 'params': params,
                    'text': 'RegNotify ev=0x%02X(%s) ct=%s' % (
                        ev, EV.get(ev, '?'), CTYPE.get(ctype, '?'))}
        return None

    return None

def parse(path):
    recs = load(path)
    t0 = recs[0]['ts']
    bufs, cid_psm, frames = {}, {}, []
    for r in recs:
        p = r['pkt']
        if not p or p[0] != 0x02:
            continue
        t = (r['ts'] - t0) / 1e6
        tx = (r['flags'] & 1) == 0
        hd = u16(p, 1)
        h = hd & 0x0FFF
        pb = (hd >> 12) & 3
        dlen = u16(p, 3)
        b = bufs.setdefault(h, bytearray())
        if pb != 0:
            del b[:]
        b += p[5:5 + dlen]
        while len(b) >= 4:
            l = u16(b, 0)
            cid = u16(b, 2)
            if len(b) < 4 + l:
                break
            pdu = bytes(b[4:4 + l])
            del b[:4 + l]
            fr = decode(cid, pdu, cid_psm, r['n'], t, tx)
            if fr:
                frames.append(fr)
    span = (recs[-1]['ts'] - t0) / 1e6
    return recs, frames, span

def fmt_meta(m):
    return "title=%-30r artist=%-16r len=%s" % (
        m.get('title'), m.get('artist'), m.get('length'))

def report(path, out):
    recs, frames, span = parse(path)
    out.write('=' * 100 + '\n')
    out.write('FILE : %s\n' % os.path.basename(path))
    out.write('信息 : records=%d  时长=%.1fs  方向: TX=手机发出  RX=仪表发来\n' % (len(recs), span))
    out.write('=' * 100 + '\n\n')

    hid = [f for f in frames if f['kind'] == 'HID']
    presses = [f for f in hid if f.get('val')]
    metas = [f for f in frames if f['kind'] == 'META']
    psts = [f for f in frames if f['kind'] == 'PLAYSTATUS']
    notifs = [f for f in frames if f['kind'] == 'NOTIFY']
    trans = [f for f in frames if f['kind'] == 'TRANSPORT']
    errs = [f for f in frames if f['kind'] in ('HIDERR', 'HIDCTL', 'HIDSET')]

    out.write('HID 按键(按下)次数: %d    按下/松开总帧: %d\n' % (len(presses), len(hid)))
    out.write('AVRCP 元数据帧: %d   播放状态帧: %d   通知帧: %d   PASS-THROUGH: %d\n' % (
        len(metas), len(psts), len(notifs), len(trans)))
    out.write('HID 异常帧(Handshake/SET_*/SUSPEND): %d %s\n\n' % (
        len(errs), ('-> ' + '; '.join('%s @%.3f' % (e['text'], e['t']) for e in errs[:5])) if errs else '(无)'))

    # ---- 逐按键窗口 ----
    for idx, pr in enumerate(presses, 1):
        t0 = pr['t']
        rel = next((h for h in hid if h['t'] > t0 and not h.get('val') and h is not pr), None)
        before = None
        for m in metas:
            if m['t'] <= t0:
                before = m
            else:
                break
        out.write('-' * 100 + '\n')
        out.write('[按键 #%d]  %s  @ %.3f s (帧 #%d)  payload=%s\n' % (
            idx, HID_KEY.get(pr['val'], hex(pr['val'])), t0, pr['n'], pr.get('raw', '')))
        out.write('          松开 @ %s\n' % ('%.3f s (帧 #%d)' % (rel['t'], rel['n']) if rel else '(未捕获)'))
        out.write('  按下前最后一条元数据: %s @ %.3f (#%d)\n' % (
            fmt_meta(before['meta']) if before else '(无)', before['t'] if before else 0, before['n'] if before else 0))

        out.write('  --- 窗口 [%.3f, %.3f] ---\n' % (t0 - 5, t0 + 5))
        lastm = lastp = None
        for f in frames:
            if not (t0 - 5 <= f['t'] <= t0 + 5):
                continue
            d = 'TX' if f['tx'] else 'RX'
            if f['kind'] == 'HID':
                out.write('     %8.3f #%-5d %s  %s\n' % (f['t'], f['n'], d, f['text']))
            elif f['kind'] == 'META':
                key = (f['meta'].get('title'), f['meta'].get('length'))
                if key != lastm:
                    out.write('     %8.3f #%-5d %s  GetElemAttr   %s\n' % (
                        f['t'], f['n'], d, fmt_meta(f['meta'])))
                lastm = key
            elif f['kind'] == 'PLAYSTATUS':
                key = (f['len'], f['status'])
                if key != lastp:
                    out.write('     %8.3f #%-5d %s  GetPlayStatus len=%-8d pos=%-8d status=%s\n' % (
                        f['t'], f['n'], d, f['len'], f['pos'], f['status']))
                lastp = key
            elif f['kind'] == 'NOTIFY' and f['ev'] in (1, 2, 5):
                out.write('     %8.3f #%-5d %s  %s params=%s\n' % (
                    f['t'], f['n'], d, f['text'],
                    ' '.join('%02X' % x for x in f['params'][:10])))
            elif f['kind'] in ('TRANSPORT', 'HIDERR', 'HIDCTL', 'HIDSET'):
                out.write('     %8.3f #%-5d %s  %s   <<< 注意\n' % (f['t'], f['n'], d, f['text']))

        # ---- 判定 ----
        chg = None
        for m in metas:
            if t0 < m['t'] <= t0 + 5:
                if before is None or (
                        (m['meta'].get('title'), m['meta'].get('length')) !=
                        (before['meta'].get('title'), before['meta'].get('length'))):
                    chg = m
                    break
        tc = [f for f in notifs if f['ev'] == 2 and t0 < f['t'] <= t0 + 5]
        if chg:
            verdict = '生效'
            why = "曲目元数据在 +%.1fs 变化: %s -> %s" % (
                chg['t'] - t0,
                before['meta'].get('title') if before else '(无)',
                chg['meta'].get('title'))
        else:
            verdict = '未生效'
            why = '按键后 5s 内手机回报的 Title/Length 没有任何变化'
        bt = (before['meta'].get('title') if before else None)
        warn = '  [注意: 前值缺失(%r), 该判定仅供参考]' % bt if bt in (None, 'Not Provided', 'Unavailable') else ''
        out.write('  >>> 判定: %s   (%s)%s\n' % (verdict, why, warn))
        if tc:
            out.write('      佐证: TRACK_CHANGED 通知 %d 条 %s\n' % (
                len(tc), ['@%.3f #%d %s' % (x['t'], x['n'], x['ctype']) for x in tc]))
        out.write('\n')

    # ---- 与按键无关的自行切歌 ----
    out.write('-' * 100 + '\n')
    out.write('>> 手机自行切歌(不在任何按键 +5s 窗口内) 的元数据变化:\n')
    prev = None
    found = False
    for m in metas:
        key = (m['meta'].get('title'), m['meta'].get('length'))
        if prev is not None and key != prev:
            if not any(0 <= m['t'] - p['t'] <= 5 for p in presses):
                out.write('     %8.3f #%-5d  %s -> %s\n' % (
                    m['t'], m['n'], prev[0], key[0]))
                found = True
        prev = key
    if not found:
        out.write('     (无)\n')
    out.write('\n')

with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'key_ctx.txt'), 'w', encoding='utf-8') as out:
    for path in sys.argv[1:]:
        report(path, out)
print('done ->', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'key_ctx.txt'))

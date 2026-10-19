import struct, sys

PAT = bytes([0x05, 0x0C, 0x09, 0x01, 0xA1, 0x01, 0x85, 0x02, 0x09, 0xCD])
TAIL = bytes([0x95, 0x01, 0x75, 0x01, 0x81, 0x01, 0xC0])

for path in sys.argv[1:]:
    data = open(path, 'rb').read()
    # 1) raw byte occurrences anywhere in the file
    raw = []
    i = 0
    while True:
        i = data.find(PAT, i)
        if i < 0:
            break
        raw.append(i)
        i += 1
    print('====', path, 'size=%d' % len(data))
    print('   raw occurrence of desc head:', len(raw), raw[:10])
    # 2) inside reassembled L2CAP PDUs
    recs = []
    off, idx = 16, 0
    while off + 24 <= len(data):
        orig, incl, flags, drops = struct.unpack_from('>IIII', data, off)
        pkt = data[off + 24: off + 24 + incl]
        off += 24 + incl
        idx += 1
        recs.append((idx, flags, pkt))
    bufs, hits = {}, []
    for n, flags, p in recs:
        if not p or p[0] != 0x02:
            continue
        hd = struct.unpack_from('<H', p, 1)[0]
        h = hd & 0xFFF
        pb = (hd >> 12) & 3
        dlen = struct.unpack_from('<H', p, 3)[0]
        b = bufs.setdefault(h, bytearray())
        if pb != 0:
            del b[:]
        b += p[5:5 + dlen]
        while len(b) >= 4:
            l = struct.unpack_from('<H', b, 0)[0]
            if len(b) < 4 + l:
                break
            pdu = bytes(b[4:4 + l])
            del b[:4 + l]
            j = pdu.find(PAT)
            if j >= 0:
                t = pdu.find(TAIL, j)
                end = (t + len(TAIL)) if t >= 0 else -1
                hits.append((n, len(pdu), j, end, len(pdu) - j if end < 0 else end - j, pdu))
    print('   in reassembled L2CAP PDU:', len(hits))
    for n, plen, j, end, seg, pdu in hits:
        print('      pkt#%-5d l2cap_pdu_len=%-4d desc_off=%-4d desc_len=%d tail_found=%s'
              % (n, plen, j, seg, end >= 0))
        print('        ctx before :', ' '.join('%02X' % x for x in pdu[max(0, j - 12):j]))
        print('        descriptor :', ' '.join('%02X' % x for x in pdu[j:j + seg]))
    if hits:
        n0 = hits[0][0]
        for n, flags, p in recs:
            if n != n0:
                continue
            print('      first hit raw pkt:', ' '.join('%02X' % x for x in p[:80]))

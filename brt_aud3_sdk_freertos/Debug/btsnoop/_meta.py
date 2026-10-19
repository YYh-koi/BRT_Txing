import struct, sys, os

def u16(b, o=0):
    return struct.unpack_from('<H', b, o)[0]

def load(path):
    data = open(path, 'rb').read()
    off, recs, idx = 16, [], 0
    while off + 24 <= len(data):
        orig, incl, flags, drops = struct.unpack_from('>IIII', data, off)
        ts = struct.unpack_from('>q', data, off + 16)[0]
        pkt = data[off + 24: off + 24 + incl]
        off += 24 + incl
        idx += 1
        recs.append((idx, flags, ts, pkt))
    return recs

TITLE = b'\x00\x00\x00\x01\x00\x6a'
TIME = b'\x00\x00\x00\x07\x00\x6a'
ARTIST = b'\x00\x00\x00\x02\x00\x6a'

def fields(pdu):
    res = {}
    for tag, key in ((TITLE, 'title'), (ARTIST, 'artist'), (TIME, 'time')):
        i = pdu.find(tag)
        if i < 0:
            continue
        ln = (pdu[i + 6] << 8) | pdu[i + 7]
        if ln > 200 or i + 8 + ln > len(pdu):
            continue
        res[key] = pdu[i + 8:i + 8 + ln].decode('utf-8', 'replace')
    return res

for path in sys.argv[1:]:
    recs = load(path)
    t0 = recs[0][2]
    bufs, out = {}, []
    for n, flags, ts, p in recs:
        if not p or p[0] != 0x02:
            continue
        hd = u16(p, 1)
        h = hd & 0xFFF
        pb = (hd >> 12) & 3
        dlen = u16(p, 3)
        b = bufs.setdefault(h, bytearray())
        if pb != 0:
            del b[:]
        b += p[5:5 + dlen]
        while len(b) >= 4:
            l = u16(b, 0)
            if len(b) < 4 + l:
                break
            pdu = bytes(b[4:4 + l])
            del b[:4 + l]
            if len(pdu) < 6 or pdu[1] != 0x11 or pdu[2] != 0x0E:
                continue
            f = fields(pdu)
            if f:
                out.append(((ts - t0) / 1e6, n, 'D>H' if (flags & 1) == 0 else 'H>D', f))
    print('====', os.path.basename(path), 'metadata frames: %d' % len(out))
    prev = None
    for t, n, dirn, f in out:
        cur = (f.get('title'), f.get('time'))
        if cur != prev:
            print('   %8.3f #%-5d %s title=%-24r time=%-8r artist=%r'
                  % (t, n, dirn, f.get('title'), f.get('time'), f.get('artist')))
        prev = cur

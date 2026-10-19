#ifndef NAT_UI_H
#define NAT_UI_H

#define NAT_SUCCESS 0

#ifdef CONFIG_NAT

UCHAR NAT_Start(
	ULONG LanIpAddr,
	UCHAR* LanMacAddr,
	ULONG WanIpAddr,
	ULONG WanSubnetMask,
	UCHAR* WanMacAddr,
	ULONG GWIpAddr,
	BOOL WanIsPPP,
	WORD PortBase,
	WORD ICMPPortBase
	);
void NAT_Stop(void);
UCHAR NAT_DealOutPacket(UCHAR* Packet, WORD len);
UCHAR NAT_DealInPacket(UCHAR* Packet, WORD len);

#else

#define NAT_Start(a, b, c, d, e, f, g, h, i) 1
#define NAT_Stop() 
#define NAT_DealOutPacket(a, b) 1
#define NAT_DealInPacket(a, b) 	1

#endif

#endif


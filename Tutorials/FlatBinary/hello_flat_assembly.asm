00000000  BE127C            mov si,0x7c12
00000003  AC                lodsb
00000004  08C0              or al,al
00000006  7406              jz 0xe
00000008  B40E              mov ah,0xe
0000000A  CD10              int 0x10
0000000C  EBF5              jmp short 0x3
0000000E  FA                cli
0000000F  F4                hlt
00000010  EBFC              jmp short 0xe
00000012  48                dec ax
00000013  656C              gs insb
00000015  6C                insb
00000016  6F                outsw
00000017  2C20              sub al,0x20
00000019  776F              ja 0x8a
0000001B  726C              jc 0x89
0000001D  642100            and [fs:bx+si],ax
00000020  0000              add [bx+si],al
00000022  0000              add [bx+si],al
00000024  0000              add [bx+si],al
00000026  0000              add [bx+si],al
00000028  0000              add [bx+si],al
0000002A  0000              add [bx+si],al
0000002C  0000              add [bx+si],al
0000002E  0000              add [bx+si],al
00000030  0000              add [bx+si],al
00000032  0000              add [bx+si],al
00000034  0000              add [bx+si],al
00000036  0000              add [bx+si],al
00000038  0000              add [bx+si],al
0000003A  0000              add [bx+si],al
0000003C  0000              add [bx+si],al
0000003E  0000              add [bx+si],al
00000040  0000              add [bx+si],al
00000042  0000              add [bx+si],al
00000044  0000              add [bx+si],al
00000046  0000              add [bx+si],al
00000048  0000              add [bx+si],al
0000004A  0000              add [bx+si],al
0000004C  0000              add [bx+si],al
0000004E  0000              add [bx+si],al
00000050  0000              add [bx+si],al
00000052  0000              add [bx+si],al
00000054  0000              add [bx+si],al
00000056  0000              add [bx+si],al
00000058  0000              add [bx+si],al
0000005A  0000              add [bx+si],al
0000005C  0000              add [bx+si],al
0000005E  0000              add [bx+si],al
00000060  0000              add [bx+si],al
00000062  0000              add [bx+si],al
00000064  0000              add [bx+si],al
00000066  0000              add [bx+si],al
00000068  0000              add [bx+si],al
0000006A  0000              add [bx+si],al
0000006C  0000              add [bx+si],al
0000006E  0000              add [bx+si],al
00000070  0000              add [bx+si],al
00000072  0000              add [bx+si],al
00000074  0000              add [bx+si],al
00000076  0000              add [bx+si],al
00000078  0000              add [bx+si],al
0000007A  0000              add [bx+si],al
0000007C  0000              add [bx+si],al
0000007E  0000              add [bx+si],al
00000080  0000              add [bx+si],al
00000082  0000              add [bx+si],al
00000084  0000              add [bx+si],al
00000086  0000              add [bx+si],al
00000088  0000              add [bx+si],al
0000008A  0000              add [bx+si],al
0000008C  0000              add [bx+si],al
0000008E  0000              add [bx+si],al
00000090  0000              add [bx+si],al
00000092  0000              add [bx+si],al
00000094  0000              add [bx+si],al
00000096  0000              add [bx+si],al
00000098  0000              add [bx+si],al
0000009A  0000              add [bx+si],al
0000009C  0000              add [bx+si],al
0000009E  0000              add [bx+si],al
000000A0  0000              add [bx+si],al
000000A2  0000              add [bx+si],al
000000A4  0000              add [bx+si],al
000000A6  0000              add [bx+si],al
000000A8  0000              add [bx+si],al
000000AA  0000              add [bx+si],al
000000AC  0000              add [bx+si],al
000000AE  0000              add [bx+si],al
000000B0  0000              add [bx+si],al
000000B2  0000              add [bx+si],al
000000B4  0000              add [bx+si],al
000000B6  0000              add [bx+si],al
000000B8  0000              add [bx+si],al
000000BA  0000              add [bx+si],al
000000BC  0000              add [bx+si],al
000000BE  0000              add [bx+si],al
000000C0  0000              add [bx+si],al
000000C2  0000              add [bx+si],al
000000C4  0000              add [bx+si],al
000000C6  0000              add [bx+si],al
000000C8  0000              add [bx+si],al
000000CA  0000              add [bx+si],al
000000CC  0000              add [bx+si],al
000000CE  0000              add [bx+si],al
000000D0  0000              add [bx+si],al
000000D2  0000              add [bx+si],al
000000D4  0000              add [bx+si],al
000000D6  0000              add [bx+si],al
000000D8  0000              add [bx+si],al
000000DA  0000              add [bx+si],al
000000DC  0000              add [bx+si],al
000000DE  0000              add [bx+si],al
000000E0  0000              add [bx+si],al
000000E2  0000              add [bx+si],al
000000E4  0000              add [bx+si],al
000000E6  0000              add [bx+si],al
000000E8  0000              add [bx+si],al
000000EA  0000              add [bx+si],al
000000EC  0000              add [bx+si],al
000000EE  0000              add [bx+si],al
000000F0  0000              add [bx+si],al
000000F2  0000              add [bx+si],al
000000F4  0000              add [bx+si],al
000000F6  0000              add [bx+si],al
000000F8  0000              add [bx+si],al
000000FA  0000              add [bx+si],al
000000FC  0000              add [bx+si],al
000000FE  0000              add [bx+si],al
00000100  0000              add [bx+si],al
00000102  0000              add [bx+si],al
00000104  0000              add [bx+si],al
00000106  0000              add [bx+si],al
00000108  0000              add [bx+si],al
0000010A  0000              add [bx+si],al
0000010C  0000              add [bx+si],al
0000010E  0000              add [bx+si],al
00000110  0000              add [bx+si],al
00000112  0000              add [bx+si],al
00000114  0000              add [bx+si],al
00000116  0000              add [bx+si],al
00000118  0000              add [bx+si],al
0000011A  0000              add [bx+si],al
0000011C  0000              add [bx+si],al
0000011E  0000              add [bx+si],al
00000120  0000              add [bx+si],al
00000122  0000              add [bx+si],al
00000124  0000              add [bx+si],al
00000126  0000              add [bx+si],al
00000128  0000              add [bx+si],al
0000012A  0000              add [bx+si],al
0000012C  0000              add [bx+si],al
0000012E  0000              add [bx+si],al
00000130  0000              add [bx+si],al
00000132  0000              add [bx+si],al
00000134  0000              add [bx+si],al
00000136  0000              add [bx+si],al
00000138  0000              add [bx+si],al
0000013A  0000              add [bx+si],al
0000013C  0000              add [bx+si],al
0000013E  0000              add [bx+si],al
00000140  0000              add [bx+si],al
00000142  0000              add [bx+si],al
00000144  0000              add [bx+si],al
00000146  0000              add [bx+si],al
00000148  0000              add [bx+si],al
0000014A  0000              add [bx+si],al
0000014C  0000              add [bx+si],al
0000014E  0000              add [bx+si],al
00000150  0000              add [bx+si],al
00000152  0000              add [bx+si],al
00000154  0000              add [bx+si],al
00000156  0000              add [bx+si],al
00000158  0000              add [bx+si],al
0000015A  0000              add [bx+si],al
0000015C  0000              add [bx+si],al
0000015E  0000              add [bx+si],al
00000160  0000              add [bx+si],al
00000162  0000              add [bx+si],al
00000164  0000              add [bx+si],al
00000166  0000              add [bx+si],al
00000168  0000              add [bx+si],al
0000016A  0000              add [bx+si],al
0000016C  0000              add [bx+si],al
0000016E  0000              add [bx+si],al
00000170  0000              add [bx+si],al
00000172  0000              add [bx+si],al
00000174  0000              add [bx+si],al
00000176  0000              add [bx+si],al
00000178  0000              add [bx+si],al
0000017A  0000              add [bx+si],al
0000017C  0000              add [bx+si],al
0000017E  0000              add [bx+si],al
00000180  0000              add [bx+si],al
00000182  0000              add [bx+si],al
00000184  0000              add [bx+si],al
00000186  0000              add [bx+si],al
00000188  0000              add [bx+si],al
0000018A  0000              add [bx+si],al
0000018C  0000              add [bx+si],al
0000018E  0000              add [bx+si],al
00000190  0000              add [bx+si],al
00000192  0000              add [bx+si],al
00000194  0000              add [bx+si],al
00000196  0000              add [bx+si],al
00000198  0000              add [bx+si],al
0000019A  0000              add [bx+si],al
0000019C  0000              add [bx+si],al
0000019E  0000              add [bx+si],al
000001A0  0000              add [bx+si],al
000001A2  0000              add [bx+si],al
000001A4  0000              add [bx+si],al
000001A6  0000              add [bx+si],al
000001A8  0000              add [bx+si],al
000001AA  0000              add [bx+si],al
000001AC  0000              add [bx+si],al
000001AE  0000              add [bx+si],al
000001B0  0000              add [bx+si],al
000001B2  0000              add [bx+si],al
000001B4  0000              add [bx+si],al
000001B6  0000              add [bx+si],al
000001B8  0000              add [bx+si],al
000001BA  0000              add [bx+si],al
000001BC  0000              add [bx+si],al
000001BE  0000              add [bx+si],al
000001C0  0000              add [bx+si],al
000001C2  0000              add [bx+si],al
000001C4  0000              add [bx+si],al
000001C6  0000              add [bx+si],al
000001C8  0000              add [bx+si],al
000001CA  0000              add [bx+si],al
000001CC  0000              add [bx+si],al
000001CE  0000              add [bx+si],al
000001D0  0000              add [bx+si],al
000001D2  0000              add [bx+si],al
000001D4  0000              add [bx+si],al
000001D6  0000              add [bx+si],al
000001D8  0000              add [bx+si],al
000001DA  0000              add [bx+si],al
000001DC  0000              add [bx+si],al
000001DE  0000              add [bx+si],al
000001E0  0000              add [bx+si],al
000001E2  0000              add [bx+si],al
000001E4  0000              add [bx+si],al
000001E6  0000              add [bx+si],al
000001E8  0000              add [bx+si],al
000001EA  0000              add [bx+si],al
000001EC  0000              add [bx+si],al
000001EE  0000              add [bx+si],al
000001F0  0000              add [bx+si],al
000001F2  0000              add [bx+si],al
000001F4  0000              add [bx+si],al
000001F6  0000              add [bx+si],al
000001F8  0000              add [bx+si],al
000001FA  0000              add [bx+si],al
000001FC  0000              add [bx+si],al
000001FE  55                push bp
000001FF  AA                stosb

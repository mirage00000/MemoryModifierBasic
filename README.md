# How To Use
For example, I know that the instruction that makes the score go down by 2 every 10 seconds in Solitaire is compiled to these bytes: 89 41 30. The 5-byte signature would be 89 41 30 E9 28.
90 in x86 assembly means "nop", instruction that does nothing. Now, lets replace the substraction instruction with "nop". Inputs for the modifier:
window title: Solitaire
byte array: 0x89 0x41 0x30 0xE9 0x28 (add 0x before the bytes) (after that press ctrl+z to indicate the end of input and press enter two times (will be fixed))
address: 0x1004b28 (the one that the search function returned)
bytes to modify: 0xe9909090 (input is 4-byte, so the last byte remains unchanged as we're changing 3-byte instruction) (the bytes order is inverse, for example if you want to change them to 01 A7 38 B2, you would write it as 0xb238a701)

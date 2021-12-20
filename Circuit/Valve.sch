EESchema Schematic File Version 4
LIBS:STLinkConverter-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J1
U 1 1 5DC89C36
P 2000 1800
F 0 "J1" H 2050 2417 50  0000 C CNN
F 1 "Conn_02x10_Odd_Even" H 2050 2326 50  0000 C CNN
F 2 "Connector_IDC:IDC-Header_2x10_P2.54mm_Vertical" H 2000 1800 50  0001 C CNN
F 3 "~" H 2000 1800 50  0001 C CNN
	1    2000 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 1700 1400 1700
Wire Wire Line
	1400 1800 1800 1800
Wire Wire Line
	1800 2100 1400 2100
Wire Wire Line
	1800 1400 1400 1400
Wire Wire Line
	1800 2000 1400 2000
NoConn ~ 1800 1500
NoConn ~ 1800 1600
NoConn ~ 1800 1900
NoConn ~ 1800 2200
NoConn ~ 1800 2300
NoConn ~ 2300 2200
NoConn ~ 2300 2100
NoConn ~ 2300 2000
NoConn ~ 2300 1900
NoConn ~ 2300 1800
NoConn ~ 2300 1700
NoConn ~ 2300 1600
NoConn ~ 2300 1400
NoConn ~ 2300 1500
Wire Wire Line
	2300 2300 2700 2300
Text Notes 1200 1000 0    50   ~ 0
STLink Conveter for STLINK-V2/1 or STLINK-V3SET.\n(ARM 20-pin JTAG/SWD to Nucleo 6-pin)
Wire Wire Line
	3500 2300 4000 2300
Wire Wire Line
	4000 2400 3600 2400
Wire Wire Line
	4000 2600 3800 2600
Wire Wire Line
	3900 1900 3900 2700
Wire Wire Line
	3900 2700 4000 2700
Wire Wire Line
	3800 2600 3800 1800
Wire Wire Line
	3700 2500 3700 1700
Wire Wire Line
	3700 2500 4000 2500
Wire Wire Line
	3600 1600 3600 2400
Wire Wire Line
	3400 2200 3400 1400
Connection ~ 3400 1400
Wire Wire Line
	3400 1400 4000 1400
Wire Wire Line
	3700 1700 4000 1700
Connection ~ 3800 1800
Wire Wire Line
	3900 1900 4000 1900
Connection ~ 3900 1900
Wire Wire Line
	4000 1800 3800 1800
Wire Wire Line
	4000 1600 3600 1600
Wire Wire Line
	3000 1400 3400 1400
Wire Wire Line
	3000 1900 3900 1900
Wire Wire Line
	3000 1800 3800 1800
Wire Wire Line
	3000 1500 3500 1500
Wire Wire Line
	3400 2200 4000 2200
Wire Wire Line
	3000 1600 3600 1600
Connection ~ 3600 1600
Wire Wire Line
	3000 1700 3700 1700
Connection ~ 3700 1700
Wire Wire Line
	3500 2300 3500 1500
Connection ~ 3500 1500
Wire Wire Line
	3500 1500 4000 1500
$Comp
L Connector_Generic:Conn_01x06 J2
U 1 1 5DC6C921
P 4200 1600
F 0 "J2" H 4280 1592 50  0000 L CNN
F 1 "Conn_01x06" H 4280 1501 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 4200 1600 50  0001 C CNN
F 3 "~" H 4200 1600 50  0001 C CNN
	1    4200 1600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x06 J3
U 1 1 5DC6C955
P 4200 2400
F 0 "J3" H 4280 2392 50  0000 L CNN
F 1 "Conn_01x06" H 4280 2301 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 4200 2400 50  0001 C CNN
F 3 "~" H 4200 2400 50  0001 C CNN
	1    4200 2400
	1    0    0    -1  
$EndComp
Text GLabel 3000 1400 0    50   Input ~ 0
VDD
Text GLabel 3000 1500 0    50   Input ~ 0
SWCLK
Text GLabel 3000 1600 0    50   Input ~ 0
GND
Text GLabel 3000 1700 0    50   Input ~ 0
SWDIO
Text GLabel 3000 1800 0    50   Input ~ 0
NRST
Text GLabel 3000 1900 0    50   Input ~ 0
SWO
Text GLabel 1400 1400 0    50   Input ~ 0
VDD
Text GLabel 1400 1700 0    50   Input ~ 0
SWDIO
Text GLabel 1400 1800 0    50   Input ~ 0
SWCLK
Text GLabel 1400 2000 0    50   Input ~ 0
SWO
Text GLabel 1400 2100 0    50   Input ~ 0
NRST
Text GLabel 2700 2300 2    50   Input ~ 0
GND
$Comp
L Device:LED_ALT D1
U 1 1 5DC83FE8
P 2250 3000
F 0 "D1" H 2242 2745 50  0000 C CNN
F 1 "LED_PWR" H 2242 2836 50  0000 C CNN
F 2 "extsui:LED_1608_HandSolderingCustom" H 2250 3000 50  0001 C CNN
F 3 "~" H 2250 3000 50  0001 C CNN
	1    2250 3000
	-1   0    0    1   
$EndComp
$Comp
L Device:R_US R1
U 1 1 5DC843B8
P 2700 3000
F 0 "R1" V 2495 3000 50  0000 C CNN
F 1 "R_10K" V 2586 3000 50  0000 C CNN
F 2 "extsui:R_1608_HandSolderingCustom" V 2740 2990 50  0001 C CNN
F 3 "~" H 2700 3000 50  0001 C CNN
	1    2700 3000
	0    1    1    0   
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 5DC84516
P 2450 3400
F 0 "SW1" H 2450 3685 50  0000 C CNN
F 1 "SW_RESET" H 2450 3594 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H4.3mm" H 2450 3600 50  0001 C CNN
F 3 "" H 2450 3600 50  0001 C CNN
	1    2450 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2550 3000 2400 3000
Wire Wire Line
	2850 3000 3050 3000
Wire Wire Line
	2100 3000 1850 3000
Wire Wire Line
	2250 3400 1850 3400
Wire Wire Line
	2650 3400 3050 3400
Text GLabel 3300 3400 2    50   Input ~ 0
GND
Text GLabel 1850 3400 0    50   Input ~ 0
NRST
Text GLabel 1850 3000 0    50   Input ~ 0
VDD
Wire Wire Line
	3050 3000 3050 3400
Connection ~ 3050 3400
Wire Wire Line
	3050 3400 3300 3400
$EndSCHEMATC

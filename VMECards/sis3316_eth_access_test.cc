/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_eth_access_test.cxx                                  */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.09.2012                                       */
/*  last modification:    11.04.2013                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Prepare IP-address on PC:                                              */
/*   Window_PC: arp -s 212.60.16.200  00-00-56-31-6x-xx                    */
/*   xxx: Serial Number                                                    */
/*                                                                         */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2013                                                                 */
/*                                                                         */
/***************************************************************************/

#define __unix__    // else WINDOWS


#include <time.h>

// need to make sure the digitizer has an IP address
// use arp to do this
// arp: address resolution protocol
// the next line is appropriate (i think) for linux
// arp -s 212.60.16.200 00:00:56:31:60:60
// for mac, use something like
// sudo arp -s 212.60.16.200 00:00:56:31:60:9a ifscope en5

//
// the MAC address for the digitizers are written on the back (for desktop versions)
// they're determined by the serial number, as well, which is how you'd determine
// the MAC address for VME versions
//


#define CERN_ROOT_PLOT

#ifdef CERN_ROOT_PLOT
#include "TApplication.h"
#include "TObject.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TMath.h"
#include "TCanvas.h"
//#include "TRandom.h"
//#include "TThread.h"
#include <TSystem.h>
#include "TLatex.h"
//#include <unistd.h>
#endif


#ifdef __unix__
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/uio.h>

#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#endif
#ifdef __APPLE__
#include <net/if.h>
#endif

#ifndef _sis3316_ethbERNETB_ACCESS_CLASS_
#include "sis3316_ethernetB_access_class.h"
#endif

#ifndef _SIS3316_CLASS_
#include "sis3316card.h"
#endif

int SIS3316_ETH_UDP_CreateUdpSocket(void); // return_value = udp_socket
int SIS3316_ETH_UDP_SetUdpSocketOptionTimeout(int udp_socket, unsigned int recv_timeout_sec, unsigned int recv_timeout_usec);
int SIS3316_ETH_UDP_SetUdpSocketOptionSetBufSize(int udp_socket, int sockbufsize);
int SIS3316_ETH_UDP_UdpSocketBindToDevice(int udp_socket, char* eth_device);
int SIS3316_ETH_UDP_UdpSocketBindMyOwnPort(int udp_socket, unsigned int udp_port);


int SIS3316_ETH_UDP_ResetCmd(int udp_socket, struct sockaddr *socket_dest_addr);
int SIS3316_ETH_UDP_RegisterWrite(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int reg_addr, unsigned int reg_data);
int SIS3316_ETH_UDP_RegisterRead(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int reg_addr, unsigned int* reg_data);

int SIS3316_ETH_UDP_SIS3316_RegisterRead(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_read_registers, unsigned int* reg_addr_ptr, unsigned int* reg_data_ptr);
int SIS3316_ETH_UDP_SIS3316_RegisterWrite(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_write_registers, unsigned int* reg_addr_ptr, unsigned int* reg_data_ptr);
int SIS3316_ETH_UDP_SIS3316_FifoRead(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_read_words, unsigned int reg_addr, unsigned int* data_ptr);
int SIS3316_ETH_UDP_SIS3316_FifoWrite(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_write_words, unsigned int fifo_addr, unsigned int* data_ptr);

int SIS3316_ETH_UDP_TestAccess(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int cmd, unsigned int length, unsigned int reg_addr, unsigned int* reg_data);

//#define  MEMORY_EVENTBLOCK_SIZE 		0x04000000  // 64 MByte
#define  MEMORY_EVENTBLOCK_SIZE 		0x0400000  // 4 MByte


/* ***************************************************************************************************************** */
//handling cmd line input
char gl_cmd_ip_string[64];
unsigned int gl_cmd_ethernet_device_no = 0;
unsigned int gl_cmd_display_cern_root  = 1 ; // display (draw with Cern-Root) is disabled
unsigned int gl_cmd_display_channel    = 0 ; // display all channels module
unsigned int gl_cmd_display_event      = 0 ; // display all events (4 Events ! )
unsigned int gl_cmd_display_max        = 0 ; // display max. Evensize of 500.000 samples (DRAW_MAX_LENGTH) -> 1000us

char gl_command[256];

unsigned int gl_rd_data[0x1000000] ; // 64 MByte
unsigned int gl_wr_data[0x1000000] ; // 64 MByte

/* ***************************************************************************************************************** */

//int _tmain(int argc, _TCHAR* argv[])
//int _tmain(int argc, char* argv[])
int main(int argc, char* argv[])
{
    
    printf("entering sis3316_eth_access_test...\n");
    int i;
    int return_code;
    
    int udp_socket;
    unsigned char *udp_recv_buffer ;
    
    unsigned int root_graph_x ;
    unsigned int root_graph_y ;
    unsigned int root_graph_x_size ;
    unsigned int root_graph_y_size ;
    unsigned int root_graph_y_offset ;
    
    char  root_graph_text[80] ;
    
    struct sockaddr_in SIS3316_sock_addr_in   ;
    
    unsigned int udp_port ;
    
    unsigned int addr, data ;
    
    unsigned int loop_counter ;
    unsigned int loop_print_counter ;
    unsigned int led_usr_U3_on_flag ;
    
    
    unsigned int udp_idle_receive_timeout_counter ;
    unsigned int udp_packet_receive_timeout_counter ;
    unsigned int udp_invalid_ack80_not_in_IdleState_counter ;
    unsigned int udp_invalid_ack81_not_in_HeaderOrDataState_counter ;
    unsigned int udp_invalid_ack82_not_in_HeaderOrDataState_counter ;
    unsigned int udp_invalid_user_packet_Header_counter ;
    unsigned int udp_block_expected_samples_error_counter ;
    unsigned int udp_packet_no_error_counter ;
    
    unsigned int block_lost_counter ;
    unsigned int old_readout_block_no ;
    unsigned int readout_block_no ;
    unsigned int header_nof_channels=0 ;
    unsigned int readout_header_info ;
    
    
    unsigned int packet_counter ;
    unsigned int readout_buffer_sample_length ;
    
    
    unsigned int jumbo_frame_flag  ;
    unsigned int packet_gap_value  ;
    
    //struct timeval struct_time;
    
    struct timeval tv;tv.tv_sec = 30;
    //const char device[] = DEVICE_ETH0 ;
    char  eth_device[64] ;
    
    
    unsigned int *SIS3316_memory_read_data ;
    
    unsigned int *read_adcx_buffer_ptr[4];
    unsigned int  readout_sample_length[4];
    
    unsigned int adc_channel_32bit_length;
    unsigned int i_ch;
    unsigned int display_max_events;
    unsigned int display_max_event_size;
    
    char ch ;
    char ch_string[64] ;
    
    clock_t time_clock_start, time_clock_finish;
    double  double_cycles_duration;
    double  double_Byte_Per_Second;
    double  double_KByte_Per_Second;
    double  double_MByte_Per_Second;
    double  duration;
    
    
    strcpy(gl_cmd_ip_string,"192.168.1.100") ;
    
    
    
    /********************************************************************************************************************************************/
    /*  Command interpreter                                                                                                                     */
    /********************************************************************************************************************************************/
    
    /* Save command line into string "command" */
    memset(gl_command,0,sizeof(gl_command));
    // memset(startchoice,0,sizeof(startchoice));
    for (i=0;i<argc;i++) {
        strcat(gl_command,argv[i]);
        strcat(gl_command," ");
    }
    
    //#ifdef raus
    if (argc > 1) {
        /* Save command line into string "command" */
        memset(gl_command,0,sizeof(gl_command));
        // memset(startchoice,0,sizeof(startchoice));
        for (i=1;i<argc;i++) {
            strcat(gl_command,argv[i]);
            strcat(gl_command," ");
        }
        printf("nof args %d    \n", argc );
        printf("gl_command %s    \n", gl_command );
        
        
        while ((ch = getopt(argc, argv, "?lhI:A:DC:E:S:")) != -1)
            //printf("ch %c    \n", ch );
            switch (ch) {
                case 'I':
                    sscanf(optarg,"%s", ch_string) ;
                    printf("-I %s    \n", ch_string );
                    strcpy(gl_cmd_ip_string,ch_string) ;
                    break;
                    
                case 'A':
                    sscanf(optarg,"%x",&data) ;
                    gl_cmd_ethernet_device_no = data ;
                    break;
                    
                case 'D':
                    gl_cmd_display_cern_root = 1 ;
                    break;
                    
                case 'C':
                    sscanf(optarg,"%d",&data) ;
                    gl_cmd_display_channel = data  ;
                    break;
                    
                case 'E':
                    sscanf(optarg,"%d",&data) ;
                    gl_cmd_display_event = data  ;
                    break;
                    
                case 'S':
                    sscanf(optarg,"%d",&data) ;
                    gl_cmd_display_max = data  ;
                    break;
                    
                case '?':
                case 'h':
                default:
                    printf("Usage: %s device  [-?h] [-I ip] [-A num]  [-D] [-C num] [-E num] [-S num] ", argv[0]);
                    //printf("[-g energyGap] [-p energyPeaking] [-s scaleFactor] [-S shiftOffset]\n");
                    printf("   \n");
                    printf("       -I string     SIS3316 IP Address                                          Default = %s\n", gl_cmd_ip_string);
                    printf("       -A num        Ethernet Device Number (0 = ETH0, 1 = ETH1, .. , 3 = ETH3)  Default = %d\n", gl_cmd_ethernet_device_no);
                    printf("   \n");
                    //	      printf("       -D            Draw enable, only possible if -R 0 (Cern-Root graphic)      Default = %d\n", gl_cmd_display_cern_root);
                    //	      printf("       -C num        Draw channel (0 = 4 channles, 1 to 4 = channel x);          Default = %d\n", gl_cmd_display_channel);
                    //	      printf("       -E num        Draw no of events (0 = 4 events, 1 = 1 (first) event);      Default = %d\n", gl_cmd_display_event);
                    //	      printf("       -S num        Draw maximum size (0 = 500.000 samples, 1 = 5000 samples);  Default = %d\n", gl_cmd_display_max);
                    printf("   \n");
                    printf("       -h            Print this message\n");
                    printf("   \n");
                    exit(1);
            }
        
    } // if (argc > 2)
    //#endif
    
    printf("gl_cmd_ip_string          = %s\n",gl_cmd_ip_string);
    printf("gl_cmd_ethernet_device_no = %d\n",gl_cmd_ethernet_device_no);
    printf("gl_cmd_display_cern_root  = %d\n",gl_cmd_display_cern_root);
    printf("gl_cmd_display_channel    = %d\n",gl_cmd_display_channel);
    printf("gl_cmd_display_event      = %d\n",gl_cmd_display_event);
    printf("gl_cmd_display_max        = %d\n",gl_cmd_display_max);
    usleep(1000);
    
    
    
    
    
    //    I don't know what exactly the point of the tests below are
    //    simple test implemented here
    //    create a digitizer object
    //    read its firmware
    //    let the rest of this stuff run and crash
    //
    //
    //
    //    note that i'm not sure what all the steps are here
    //    this is mostly copied from tino's example code
    //
    
    sis3316_ethb* sis3316card_interface = new sis3316_ethb();
    int	sockbufsize = 335544432 ; // 0x2000000
    return_code = sis3316card_interface->set_UdpSocketOptionBufSize(sockbufsize) ;
    
    unsigned int nof_found_devices;
    CHAR char_messages[256];
    char  pc_ip_addr_string[32] ;
    char  sis3316_ip_addr_string[32] ;
    strcpy(sis3316_ip_addr_string,"192.168.1.100") ; // SIS3316 IP address
    //int return_code ;
    strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
    return_code = sis3316card_interface->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);
    
    sis3316card_interface->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string);
    
    sis3316card_interface->udp_reset_cmd();
    
    // open Vme Interface device
    return_code = sis3316card_interface->vmeopen ();  // open Vme interface
    sis3316card_interface->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
    
    printf("Ethernet interface opened and found number of devices %i with messages:\n%s\n", nof_found_devices, char_messages );
    
    
    //    now make our card
    sis3316card* sis3316 = new sis3316card(sis3316card_interface, 0x0);
    
    //    attempt to read firmware id
    printf("attempting to read digitizer information (module ID register)\n");
    return_code = sis3316->vmei->vme_A32D32_read(SIS3316_MODID, &data );
    printf("read had return code %i, returned value %08x\n", return_code, data );
    /********************************************************************************************************************************************/
    /*  Create and Setup UDP Socket                                                                                                             */
    /********************************************************************************************************************************************/
    
    /* Construct the SIS3316 sockaddr_in structure)  */
    memset(&SIS3316_sock_addr_in, 0, sizeof(SIS3316_sock_addr_in));
    strcpy(eth_device,"eth1") ;
    //SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr("212.60.16.200");
    SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr(gl_cmd_ip_string);
    
    udp_port = 0xE000 ;
    
    switch (gl_cmd_ethernet_device_no) {
        case 0:
            strcpy(eth_device,"eth0") ;
            //SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr("212.60.16.200");
            udp_port = 0xE000 ;
            break;
        case 1:
            strcpy(eth_device,"eth1") ;
            //SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr("213.60.16.200");
            udp_port = 0xD001 ;
            break;
        case 2:
            strcpy(eth_device,"eth2") ;
            //SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr("214.60.16.200");
            udp_port = 0xD002 ;
            break;
        case 3:
            strcpy(eth_device,"eth3") ;
            //SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr("215.60.16.200");
            udp_port = 0xD003 ;
            break;
        default:
            break;
    }
    // prepare ethernet connection
    SIS3316_sock_addr_in.sin_family = AF_INET;
    SIS3316_sock_addr_in.sin_port = htons(udp_port);
    //SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr("213.60.16.201");
    //SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr("127.0.0.201");
    memset(&(SIS3316_sock_addr_in.sin_zero),0,8);
    //bzero(&(SIS3316_sock_addr_in.sin_zero),8);
    
    // Create udp_socket
    do {
        if ((udp_socket = SIS3316_ETH_UDP_CreateUdpSocket()) == -1) {
            printf("Error: SIS3316_ETH_UDP_CreateUdpSocket\n");
        }
        // set Receive Timeout to 500ms
        return_code = SIS3316_ETH_UDP_SetUdpSocketOptionTimeout(udp_socket, 0 /* sec */, 500000 /* usec */); // 500ms
        if (return_code == -1) {
            printf("Error: SIS3316_ETH_UDP_SetUdpSocketOptionTimeout\n");
        }
        
        // increase read_buffer size
        // SUSE needs following command as su: >sysctl -w net.core.rmem_max=33554432
        int sockbufsize = 335544432 ; // 0x2000000
        return_code = SIS3316_ETH_UDP_SetUdpSocketOptionSetBufSize(udp_socket, sockbufsize); //
        if (return_code == -1) {
            printf("Error: SIS3316_ETH_UDP_SetUdpSocketOptionSetBufSize\n");
        }
        
        // needs to be superuser
        // bind the udp_socket to one network device
        return_code = SIS3316_ETH_UDP_UdpSocketBindToDevice(udp_socket, eth_device); //
        if (return_code == -1) {
            printf("Error: SIS3316_ETH_UDP_UdpSocketBindToDevice\n");
        }
        
        return_code = SIS3316_ETH_UDP_UdpSocketBindMyOwnPort(udp_socket, udp_port); //
        if (return_code == -1) {
            printf("Error: SIS3316_ETH_UDP_UdpSocketBindMyOwnPort   udp_socket = 0x%08x \n",udp_socket);
            udp_port++;
            SIS3316_sock_addr_in.sin_port = htons(udp_port);
            printf("Try with new Port again   udp_port = 0x%08x \n",udp_port);
        }
    } while (return_code != 0) ;
    
    
    
    
    //#ifdef raus
    
    // Hinweis: wenn UDP port schon benutzt wird, gibt es einen Fehler !!
    // Fehler in SIS3316_ETH_UDP_UdpSocketBindMyOwnPort auswerten und gegebenfalls neue Port addresse suchen
    // mit "netstat -a" auf console ueberpruefen
    //#endif
    
    
    /********************************************************************************************************************************************/
    /*  malloc buffers                                                                                                                           */
    /********************************************************************************************************************************************/
    
    udp_recv_buffer = (unsigned char*)malloc(0x4000); // 16384
    if(udp_recv_buffer == NULL) {
        printf("Error: malloc:udp_recv_buffer\n");
        exit(1);
    }
    
    SIS3316_memory_read_data = (unsigned int*)malloc(MEMORY_EVENTBLOCK_SIZE);
    if(SIS3316_memory_read_data == NULL) {
        printf("Error: malloc:SIS3316_memory_read_data\n");
        exit(1);
    }
    
    /********************************************************************************************************************************************/
    /*  Reset and check UDP Connection                                                                                                          */
    /********************************************************************************************************************************************/
    
    SIS3316_ETH_UDP_ResetCmd(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in);
    unsigned int  timeout_counter;
    unsigned int  print_counter;
 	  unsigned int  led_flag;
    led_flag = 1 ;
    print_counter = 0 ;
    timeout_counter = 0 ;
    loop_counter = 0 ;
    // check UDP Connection
    
    //    read the register with model information
    //    just to confirm we're talking constructively..
    unsigned char tempByte[4];
    printf("checking module information register...\n");
    return_code = SIS3316_ETH_UDP_RegisterRead(udp_socket, (struct sockaddr*)&SIS3316_sock_addr_in, 0x4, (unsigned int*)tempByte);
    printf("read module register, got data %08x with return code %i\n", (unsigned int)*tempByte, return_code );
    
    
#define tests
#ifdef tests
    unsigned int read_register_length ;
    unsigned int read_address_array[64] ;
    unsigned int read_data_array[64] ;
    unsigned int write_register_length ;
    unsigned int write_address_array[64] ;
    unsigned int write_data_array[64] ;
    
    time_clock_start = clock();
    //do {
    for (i = 0;i<10000;i++) {
        SIS3316_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, 0x10, 0x1); //
    }
    time_clock_finish = clock();
    printf( "start = %2.3f seconds\n", (double)(time_clock_start)/CLOCKS_PER_SEC );
    printf( "finish = %2.3f seconds\n", (double)(time_clock_finish) / CLOCKS_PER_SEC );
    
    double_cycles_duration = (double)(time_clock_finish - time_clock_start) / CLOCKS_PER_SEC ;
    printf( "%2.3f seconds\n", double_cycles_duration );
    
    double_Byte_Per_Second =  (double)(10000 * 4) / double_cycles_duration ;
    double_KByte_Per_Second =  double_Byte_Per_Second / 1000 ;
    printf( "%2.1f transfer speed \n", double_KByte_Per_Second );
    
    printf( "sis3150Usb_Register_Single_Write: %2.3f KByte/sec    cycle rep. = %2.3f ms \n",
           double_KByte_Per_Second, 1/double_KByte_Per_Second );
    
    
    // write
    write_register_length  = 1 ;
    write_address_array[0] = 0x08 ;
    write_data_array[0]    = 0x2 ;
    return_code = SIS3316_ETH_UDP_SIS3316_RegisterWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, write_register_length, write_address_array, write_data_array);
    
    write_register_length  = 1 ;
    write_address_array[0] = 0x80 ;
    write_data_array[0]    = 0xC0000000 ;
    return_code = SIS3316_ETH_UDP_SIS3316_RegisterWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, write_register_length, write_address_array, write_data_array);
    
    for (i=0;i<0x10000;i++) {
        gl_wr_data[i] = 0x11000000 + i;
    }
    
    unsigned int write_fifo_length ;
    write_fifo_length = 0x100;
    
    for (i=0;i<0x10;i++) {
        return_code = SIS3316_ETH_UDP_SIS3316_FifoWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, write_fifo_length, 0x100000, &gl_wr_data[i*write_fifo_length]);
    }
    
    
    for (i=0;i<0x10000;i++) {
        gl_rd_data[i] = 0x55555555;
    }
    
    do {
        write_register_length  = 1 ;
        write_address_array[0] = 0x80 ;
        write_data_array[0]    = 0x80000000 ;
        return_code = SIS3316_ETH_UDP_SIS3316_RegisterWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, write_register_length, write_address_array, write_data_array);
        
        //usleep(1000) ;
        unsigned int read_fifo_length ;
        unsigned int error_counter ;
        read_fifo_length  = 0x10000 ;
        error_counter  = 0 ;
        print_counter++ ;
        loop_counter++ ;
        return_code = SIS3316_ETH_UDP_SIS3316_FifoRead(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, read_fifo_length, 0x100000, gl_rd_data);
#ifdef raus
        if (return_code == 0) {
            for (i=0;i<read_fifo_length;i++) {
                if (gl_wr_data[i] != gl_rd_data[i]) {
                    printf("i = 0x%08x   gl_wr_data = 0x%08x   gl_rd_data = 0x%08x   \n",i, gl_wr_data[i], gl_rd_data[i]);
                    error_counter++;
                    if (error_counter > 10) {
                        break ;
                    }
                }
            }
        }
#endif
        print_counter++;
        loop_counter++;
        if(print_counter == 1000) {
            printf("loop_counter =  %d   \n", loop_counter);
            print_counter = 0 ;
        }
        
        //return_code = SIS3316_ETH_UDP_SIS3316_FifoRead(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, read_register_length, 0x100000, &gl_rd_data[read_register_length]);
        //for (i=0;i<read_register_length;i++) {
        //	printf("read_data_array:  0x%08x   \n",gl_rd_data[i]);
        //}
        //usleep(1000) ;
        
        
    } while (1) ;
    do {
#ifdef raus
        usleep(1000) ;
        print_counter++ ;
        loop_counter++ ;
        read_register_length  = 4 ;
        read_address_array[0] = 0x1000 ;
        read_address_array[1] = 0x1004 ;
        read_address_array[2] = 0x1008 ;
        read_address_array[3] = 0x100C ;
        //return_code = SIS3316_ETH_UDP_SIS3316_RegisterRead(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, read_register_length, read_address_array, read_data_array);
#endif
        write_register_length  = 8 ;
        write_address_array[0] = 0x1000 ;
        write_address_array[1] = 0x1004 ;
        write_address_array[2] = 0x1008 ;
        write_address_array[3] = 0x100C ;
        write_address_array[4] = 0x1010 ;
        write_address_array[5] = 0x1014 ;
        write_address_array[6] = 0x1018 ;
        write_address_array[7] = 0x101C ;
        write_data_array[0] = 0x12345678 ;
        write_data_array[1] = 0x3456789A ;
        write_data_array[2] = 0x56789ABC ;
        write_data_array[3] = 0x789ABCDE ;
        write_data_array[4] = 0x12345678 ;
        write_data_array[5] = 0x3456789A ;
        write_data_array[6] = 0x56789ABC ;
        write_data_array[7] = 0x789ABCDE ;
        return_code = SIS3316_ETH_UDP_SIS3316_RegisterWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, write_register_length, write_address_array, write_data_array);
        
        
        //usleep(1000) ;
        //return_code = SIS3316_ETH_UDP_TestAccess( udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, 0x20, 0, 0x00000028, &data);
    } while (1) ;
    
#endif
    
    
    do {
        print_counter++ ;
        loop_counter++ ;
        
        return_code = SIS3316_ETH_UDP_RegisterRead( udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, 0x4, &data);
        if (return_code == 0) {
            if(print_counter == 10000) {
                if (led_flag == 1) {
                    led_flag = 0 ;
                    SIS3316_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, 0x0, 0x0); // clr Led
                }
                else {
                    led_flag = 1 ;
                    SIS3316_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&SIS3316_sock_addr_in, 0x0, 0x1); // set Led
                }
                
                print_counter = 0 ;
                printf("Module and Version register:  0x%08x   loop_counter = %d     timeout_counter  = %d \n",data, loop_counter, timeout_counter);
            }
        }
        else {
            timeout_counter++;
            if (return_code == -1) {
                printf("Module and Version register: read timeout %d \n",return_code);
            }
            else {
                printf("Module and Version register: wrong read length (must be 9) %d \n",return_code);
            }
        }
        
    } while (1) ;
    //  } while (return_code != 0) ;
    
    
    return 0;
}









/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */



/* ***************************************************************************************************************** */
/* Ethernet-UDP routines                                                                                             */
/* ***************************************************************************************************************** */

/* ***************************************************************************************************************** */

int SIS3316_ETH_UDP_CreateUdpSocket(void)
{
    int return_code;
    return_code = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return return_code;
}

/* ***************************************************************************************************************** */

int SIS3316_ETH_UDP_SetUdpSocketOptionTimeout(int udp_socket, unsigned int recv_timeout_sec, unsigned int recv_timeout_usec)
{
    int return_code;
    
#ifdef __unix__
    printf("Setting UDP Timeout\n");
    struct timeval struct_time;
    
    struct_time.tv_sec  = recv_timeout_sec;
    struct_time.tv_usec = recv_timeout_usec; //
    
    return_code = setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&struct_time,sizeof(struct timeval));
#endif
    
    
    if (return_code == -1) {
        printf("Error: SIS3316_ETH_UDP_SetUdpSocketOptionTimeout \n");
    }
    return return_code;
}

/* ***************************************************************************************************************** */


int SIS3316_ETH_UDP_SetUdpSocketOptionSetBufSize(int udp_socket, int sockbufsize)
{
    int return_code;
    unsigned char recv_data[16];
    socklen_t addr_len;
    
    return_code = setsockopt(udp_socket, SOL_SOCKET,SO_RCVBUF, (char *) &sockbufsize, (int)sizeof(sockbufsize));
    if (return_code == -1) {
        printf("Error: SIS3316_ETH_UDP_SetUdpSocketOptions, setsockopt\n");
        return return_code;
    }
    
    return_code = getsockopt(udp_socket, SOL_SOCKET,SO_RCVBUF, &recv_data, &addr_len);
    if (return_code == -1) {
        printf("Error: SIS3316_ETH_UDP_SetUdpSocketOptions, getsockopt\n");
    }
    return return_code;
}


/* ***************************************************************************************************************** */

// must be call as superuser
int SIS3316_ETH_UDP_UdpSocketBindToDevice(int udp_socket, char* eth_device)
{
    int return_code=0;
    
#ifdef __linux__
    return_code = setsockopt(udp_socket, SOL_SOCKET, SO_BINDTODEVICE, eth_device, sizeof(eth_device));
    if (return_code == -1) {
        printf("Error: SIS3316_ETH_UDP_UdpSocketBindToDevice \n");
    }
#elif __APPLE__
    int idx = if_nametoindex(eth_device);
    return_code = setsockopt(udp_socket, SOL_SOCKET, IP_BOUND_IF, &idx, sizeof(idx) );
#endif
    return return_code;
}


/* ***************************************************************************************************************** */
//int SIS3316_ETH_UDP_UdpSocketBindMyOwnPort(int udp_socket, char* eth_device, char* eth_ip_string, unsigned int udp_port, struct sockaddr *socket_dest_addr)

// must be call as superuser
int SIS3316_ETH_UDP_UdpSocketBindMyOwnPort(int udp_socket, unsigned int udp_port)
{
    int return_code;
    struct sockaddr_in my_addr;
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(udp_port);
    my_addr.sin_addr.s_addr = 0x0 ; //ADDR_ANY;
    memset(&(my_addr.sin_zero),0,8);
    //bzero(&(my_addr.sin_zero),8);
    
    
    return_code = bind(udp_socket,(struct sockaddr *)&my_addr, sizeof(my_addr));
    if (return_code == -1) {
        printf("Error: SIS3316_ETH_UDP_UdpSocketBindMyOwnPort \n");
    }
    return return_code;
}




/* ***************************************************************************************************************** */

int SIS3316_ETH_UDP_ResetCmd(int udp_socket, struct sockaddr *socket_dest_addr)
{
    int return_code;
    char send_data[4];
    // write Cmd
    send_data[0] = 0xFF ; // reset
    return_code = sendto(udp_socket, send_data, 1, 0, socket_dest_addr, sizeof(struct sockaddr));
    
    return return_code;
}

/* ***************************************************************************************************************** */
int SIS3316_ETH_UDP_RegisterWrite(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int reg_addr, unsigned int reg_data)
{
    int return_code;
    char send_data[16];
    // write to register
    send_data[0] = 0x11 ; // register write CMD
    send_data[1] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
    send_data[2] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
    send_data[3] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
    send_data[4] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
    send_data[5] = (unsigned char)  (reg_data & 0xff) ;        // data(7 dwonto 0)
    send_data[6] = (unsigned char) ((reg_data >>  8) & 0xff) ; // data(15 dwonto 8)
    send_data[7] = (unsigned char) ((reg_data >> 16) & 0xff) ; // data(23 dwonto 16)
    send_data[8] = (unsigned char) ((reg_data >> 24) & 0xff) ; // data(31 dwonto 24)
    return_code = sendto(udp_socket, send_data, 9, 0, socket_dest_addr, sizeof(struct sockaddr));
    
    return return_code;
}

/* ***************************************************************************************************************** */

int SIS3316_ETH_UDP_RegisterRead(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int reg_addr, unsigned int* reg_data)
{
    int return_code;
    int receive_bytes;
    unsigned int *data_ptr;
    //unsigned int data;
    char send_data[16];
    char recv_data[16];
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);
    
    // send Read Req Cmd
    send_data[0] = 0x10 ; // register read CMD
    send_data[1] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
    send_data[2] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
    send_data[3] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
    send_data[4] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
    return_code = sendto(udp_socket, send_data, 5, 0, socket_dest_addr, sizeof(struct sockaddr));
    
    //receive_bytes = read(socket,recv_data,9);
    receive_bytes = recvfrom(udp_socket, recv_data, 9, 0,  socket_dest_addr, &addr_len);
    
    if(receive_bytes == 9) {
        data_ptr = (unsigned int*)&recv_data[5];
        //data = *data_ptr;
        *reg_data = *data_ptr;
        //printf("receive_bytes  %d \n",receive_bytes);
        //printf("register read  0x%08x \n",data);
        return 0;
    }
    
    return receive_bytes;
    // -1: timeout, > 0; wrong number in receive_bytes
}


/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */

int SIS3316_ETH_UDP_SIS3316_RegisterRead(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_read_registers, unsigned int* reg_addr_ptr, unsigned int* reg_data_ptr)
{
    int i;
    int return_code;
    int receive_bytes;
    unsigned int nof_32bit_word;
    unsigned int reg_addr, reg_data;
    unsigned int *data_ptr;
    //unsigned int data;
    char send_data[512];
    char recv_data[512];
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);
    
    nof_32bit_word = nof_read_registers ;
    if (nof_read_registers == 0) {
        nof_32bit_word = 1 ;
    }
    if (nof_read_registers > 64) {
        nof_32bit_word = 64 ;
    }
    
    send_data[0] = 0x20 ; // send SIS3316 Register Read Req Cmd
    send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length
    for (i=0;i<nof_32bit_word;i++) {
        reg_addr = reg_addr_ptr[i] ;
        send_data[(4*i)+3] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
        send_data[(4*i)+4] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
        send_data[(4*i)+5] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
        send_data[(4*i)+6] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
    }
    return_code = sendto(udp_socket, send_data, 3 + (4*nof_32bit_word), 0, socket_dest_addr, sizeof(struct sockaddr));
    
    //receive_bytes = read(socket,recv_data,9);
    receive_bytes = recvfrom(udp_socket, recv_data, 512, 0,  socket_dest_addr, &addr_len);
    printf("receive_bytes  %d \n",receive_bytes);
    
    //		  memcpy(&uint_adc_data_buffer[udp_data_copy_to_buffer_index], &recv_data[2], receive_bytes-2) ;
    
    if(receive_bytes == (3 + (4*nof_32bit_word))) {
        for (i=0;i<nof_32bit_word;i++) {
            data_ptr = (unsigned int *)&recv_data[3+(4*i)] ;
            reg_data_ptr[i] = *data_ptr ;
            printf("register read  0x%08x \n",reg_data_ptr[i]);
        }
        return 0;
    }
    
    return receive_bytes;
    // -1: timeout, > 0; wrong number in receive_bytes
}

/* ***************************************************************************************************************** */

int SIS3316_ETH_UDP_SIS3316_RegisterWrite(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_write_registers, unsigned int* reg_addr_ptr, unsigned int* reg_data_ptr)
{
    int i;
    int return_code;
    int receive_bytes;
    unsigned int nof_32bit_word;
    unsigned int reg_addr, reg_data;
    unsigned int *data_ptr;
    //unsigned int data;
    char send_data[512];
    char recv_data[512];
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);
    
    nof_32bit_word = nof_write_registers ;
    if (nof_write_registers == 0) {
        nof_32bit_word = 1 ;
    }
    if (nof_write_registers > 64) {
        nof_32bit_word = 64 ;
    }
    
    send_data[0] = 0x21 ; // send SIS3316 Register Read Req Cmd
    send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length
    for (i=0;i<nof_32bit_word;i++) {
        reg_addr = reg_addr_ptr[i] ;
        send_data[(8*i)+3] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
        send_data[(8*i)+4] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
        send_data[(8*i)+5] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
        send_data[(8*i)+6] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
        reg_data = reg_data_ptr[i] ;
        send_data[(8*i)+7]  = (unsigned char)  (reg_data & 0xff) ;        // reg_data(7 dwonto 0)
        send_data[(8*i)+8]  = (unsigned char) ((reg_data >>  8) & 0xff) ; // reg_data(15 dwonto 8)
        send_data[(8*i)+9]  = (unsigned char) ((reg_data >> 16) & 0xff) ; // reg_data(23 dwonto 16)
        send_data[(8*i)+10] = (unsigned char) ((reg_data >> 24) & 0xff) ; // reg_data(31 dwonto 24)
    }
    return_code = sendto(udp_socket, send_data, 3 + (8*nof_32bit_word), 0, socket_dest_addr, sizeof(struct sockaddr));
    
    //receive_bytes = read(socket,recv_data,9);
    receive_bytes = recvfrom(udp_socket, recv_data, 512, 0,  socket_dest_addr, &addr_len);
    //	printf("receive_bytes  %d \n",receive_bytes);
    
    //		  memcpy(&uint_adc_data_buffer[udp_data_copy_to_buffer_index], &recv_data[2], receive_bytes-2) ;
    
    if(receive_bytes ==  3 ) {
        return 0;
    }
    
    return receive_bytes;
    // -1: timeout, > 0; wrong number in receive_bytes
}





/* ***************************************************************************************************************** */

int SIS3316_ETH_UDP_SIS3316_FifoRead(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_read_words, unsigned int reg_addr, unsigned int* data_ptr)
{
    int i;
    int return_code;
    int receive_bytes;
    int send_length;
    unsigned int nof_32bit_word;
    unsigned int reg_data;
    //unsigned int data;
    char send_data[512];
    char recv_data[2048];
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);
    
    nof_32bit_word = nof_read_words ;
    if (nof_read_words == 0) {
        nof_32bit_word = 1 ;
    }
    if (nof_read_words > 0x10000) {
        nof_32bit_word = 0x10000 ;
    }
    
    send_data[0] = 0x30 ; // send SIS3316 Register Read Req Cmd
    send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length
    send_data[3] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
    send_data[4] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
    send_data[5] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
    send_data[6] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
    send_length = 7 ;
    return_code = sendto(udp_socket, send_data, send_length, 0, socket_dest_addr, sizeof(struct sockaddr));
    if (return_code != send_length) {
        printf("Error: sendto return_code =  %d   \n",return_code);
    }
    //receive_bytes = read(socket,recv_data,9);
    unsigned int udp_data_copy_to_buffer_index ;
    unsigned int nof_read_data_bytes ;
    int rest_length_byte ;
    rest_length_byte = 4 * nof_32bit_word ;
    udp_data_copy_to_buffer_index = 0 ;
    //printf("rest_length_byte  %d   \n",rest_length_byte);
    
    unsigned int soft_packet_number;
    unsigned int packet_number;
    unsigned char* uchar_ptr;
    uchar_ptr = (unsigned char* ) data_ptr ;
    packet_number = 0;
    soft_packet_number = 0;
    do {
        receive_bytes = recvfrom(udp_socket, recv_data, 2000, 0,  socket_dest_addr, &addr_len);
        if (receive_bytes == -1) {
            printf("Error: recvfrom receive_bytes =  %d   \n",receive_bytes);
            printf("receive_bytes  %d   Ack = %2x  Status = %2x  data = %2x \n",receive_bytes, (unsigned char) recv_data[0], (unsigned char) recv_data[1], (unsigned char) recv_data[2]);
            printf("soft_packet_number  %d   \n",soft_packet_number);
            printf("udp_data_copy_to_buffer_index  %d   \n",udp_data_copy_to_buffer_index);
            return -1 ;
            //break ;
        }
        soft_packet_number++;
        
        printf("receive_bytes  %d   Ack = %2x  Status = %2x  data = %2x \n",receive_bytes,(unsigned char)  recv_data[0], (unsigned char) recv_data[1], (unsigned char) recv_data[2]);
        //printf("udp_data_copy_to_buffer_index  %d   \n",udp_data_copy_to_buffer_index);
        
        nof_read_data_bytes = receive_bytes-2  ;
        memcpy(&uchar_ptr[udp_data_copy_to_buffer_index], &recv_data[2], nof_read_data_bytes) ;
        udp_data_copy_to_buffer_index = udp_data_copy_to_buffer_index +  nof_read_data_bytes;
        
        rest_length_byte = rest_length_byte - nof_read_data_bytes ;
        
        //printf("rest_length_byte  %d   \n",rest_length_byte);
        
    } while (rest_length_byte > 0) ;
    printf("end \n");
    return 0;
    // -1: timeout, > 0; wrong number in receive_bytes
}


int SIS3316_ETH_UDP_SIS3316_FifoWrite(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int nof_write_words, unsigned int fifo_addr, unsigned int* data_ptr)
{
    int i;
    int return_code;
    int receive_bytes;
    int send_length;
    unsigned int nof_32bit_word;
    unsigned int data;
    //unsigned int data;
    char send_data[2048];
    char recv_data[2048];
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);
    
    nof_32bit_word = nof_write_words ;
    if (nof_write_words == 0) {
        nof_32bit_word = 1 ;
    }
    if (nof_write_words > 0x100) {
        nof_32bit_word = 0x100 ;
    }
    
    send_data[0] = 0x31 ; // send SIS3316 Register Read Req Cmd
    send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length
    send_data[3] = (unsigned char)  (fifo_addr & 0xff) ;        // address(7 dwonto 0)
    send_data[4] = (unsigned char) ((fifo_addr >>  8) & 0xff) ; // address(15 dwonto 8)
    send_data[5] = (unsigned char) ((fifo_addr >> 16) & 0xff) ; // address(23 dwonto 16)
    send_data[6] = (unsigned char) ((fifo_addr >> 24) & 0xff) ; // address(31 dwonto 24)
    send_length = 7 + (4*nof_32bit_word);
    for (i=0;i<nof_32bit_word;i++) {
        data = data_ptr[i] ;
        send_data[7+(4*i)] = (unsigned char)  (data & 0xff) ;        // address(7 dwonto 0)
        send_data[8+(4*i)] = (unsigned char) ((data >>  8) & 0xff) ; // address(15 dwonto 8)
        send_data[9+(4*i)] = (unsigned char) ((data >> 16) & 0xff) ; // address(23 dwonto 16)
        send_data[10+(4*i)] = (unsigned char) ((data >> 24) & 0xff) ; // address(31 dwonto 24)
    }
    return_code = sendto(udp_socket, send_data, send_length, 0, socket_dest_addr, sizeof(struct sockaddr));
    if (return_code != send_length) {
        printf("Error: sendto return_code =  %d   \n",return_code);
    }
    //receive_bytes = read(socket,recv_data,9);
    receive_bytes = recvfrom(udp_socket, recv_data, 2000, 0,  socket_dest_addr, &addr_len);
    if (receive_bytes == -1) {
        printf("Error: recvfrom receive_bytes =  %d   \n",receive_bytes);
    }
    
    printf("receive_bytes  %d   Ack = %2x  Status = %2x \n",receive_bytes, (unsigned char) recv_data[0], (unsigned char) recv_data[1]);
    
    return receive_bytes;
    // -1: timeout, > 0; wrong number in receive_bytes 
}



int SIS3316_ETH_UDP_TestAccess(int udp_socket, struct sockaddr *socket_dest_addr, unsigned int cmd, unsigned int length, unsigned int reg_addr, unsigned int* reg_data)
{
    unsigned int data;
    int i;
    int return_code;
    int receive_bytes;
    unsigned int *data_ptr;
    unsigned int length_32bit;
    char send_data[128];
    char recv_data[128];
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);
    
    unsigned int temp;
    temp = reg_addr ;
    for (i=0;i<10;i++) {
        send_data[3 + 0 + (4*i)] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
        send_data[3 + 1 + (4*i)] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
        send_data[3 + 2 + (4*i)] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
        send_data[3 + 3 + (4*i)] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
        temp = temp+4 ;
    }
    
    
    length_32bit = length + 1 ;
    // send Read Req Cmd
    //send_data[0] = 0x10 ; // register read CMD
    send_data[0] = (unsigned char)  (cmd & 0xff) ;   // sis3316 register space read CMD
    send_data[1] = (unsigned char)  (length & 0xff);        //  lower length
    send_data[2] = (unsigned char) ((length >>  8) & 0xff);        //  upper length 
    send_data[3] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
    send_data[4] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
    send_data[5] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
    send_data[6] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
    return_code = sendto(udp_socket, send_data, 3 + (4 * length_32bit), 0, socket_dest_addr, sizeof(struct sockaddr));
    
    //receive_bytes = read(socket,recv_data,9);
    receive_bytes = recvfrom(udp_socket, recv_data, 1000, 0,  socket_dest_addr, &addr_len);
    printf("receive_bytes  %d \n",receive_bytes);
    
    //if(receive_bytes == 9) {
    data_ptr = (unsigned int*)&recv_data[0];
    //data = *data_ptr;
    printf("read  ");
    for (i=0;i<receive_bytes;i++) {
        printf("%02x ", recv_data[i]);
    }
    printf("\n");
    data_ptr = (unsigned int*)&recv_data[3];
    *reg_data = *data_ptr;
    
    data = *data_ptr;
    printf("register read  0x%08x \n",data);
    
    return 0;
    // }
    
    return receive_bytes;
    // -1: timeout, > 0; wrong number in receive_bytes 
}



/* ***************************************************************************************************************** */

#ifdef WINDOWS
void usleep(unsigned int uint_usec) 
{
    unsigned int msec;
    if (uint_usec <= 1000) {
        msec = 1 ;
    }
    else {
        msec = (uint_usec+999) / 1000 ;
    }
    Sleep(msec);
    
}
#endif


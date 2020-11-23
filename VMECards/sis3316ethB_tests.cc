#include "vme_interface_class.h"
#include "stdio.h"
#include "TStopwatch.h"
#include "sis3316card.h"
#include "sis3316_ethernetB_access_class.h"



// need to make sure the digitizer has an IP address
// use arp to do this
// arp: address resolution protocol
// the next line is appropriate (i think) for linux
// arp -s 212.60.16.200 00:00:56:31:60:60
// for mac, use something like
// sudo arp -s 212.60.16.200 00:00:56:31:60:9a ifscope en5
//or
// sudo arp -s 212.60.16.200 00:00:56:31:60:9c ifscope en4

//
// the MAC address for the digitizers are written on the back (for desktop versions)
// they're determined by the serial number, as well, which is how you'd determine
// the MAC address for VME versions
//



// call from command line
//BY DEFAULT, if no arguments passed, assume
//  OUR IP address is 212.60.16.199
//  DIGITIZER IP address is 212.60.16.200
// if arugments are supplied
//  1st arg: our IP address
//  2nd arg: digitizer IP address
//int sis3316ethB_tests()
int main( int argc, char* argv[] ) {
    
//    char* used for IP addresses
//    step added, rather than using string literals, because C++11 no longer likes conversions between literals and char*s
    char ipAddress_ours[32];
    char ipAddress_digi[32];
    if( argc == 1 ) {
//        no arguments provided
//        use default IP addresses
        strcpy( ipAddress_ours,  "192.168.2.99" );
        strcpy( ipAddress_digi,  "192.168.2.100" );
    }
    else if( argc == 3 ) {
        sprintf( ipAddress_ours, "%s", argv[1] );
        sprintf( ipAddress_digi, "%s", argv[2] );
    }
    else {
        printf("ERROR incorrect number of arguments passed (%i)\n", argc);
        printf("Either supply no arguments to use default IP addresses (digitizer 212.60.16.200, computer 212.60.16.199)\n");
        printf("Or supply two arguments, 1st specifying own IP, 2nd specifying digitizer IP\n");
        return -1;
    }
    printf( "Running tests with digitizer IP %s using computer IP of %s\n", ipAddress_digi, ipAddress_ours );
    
    
    
    
    
    
    
    unsigned int nof_found_devices;
    CHAR char_messages[256];
    int return_code = 0;
    unsigned int baseAddress =0x00000000;
    unsigned int addr =0x0;
    unsigned int* buff = new unsigned int[0x400000];
    unsigned int got_nof_32bit_words =0;
    unsigned int data;
    
    
    
    
    
    sis3316_ethb* vmei = new sis3316_ethb();
    int	sockbufsize = 335544432 ; // 0x2000000
    return_code = vmei->set_UdpSocketOptionBufSize(sockbufsize) ;
    
//    unsigned int nof_found_devices;
//    CHAR char_messages[256];
    char  pc_ip_addr_string[32] ;
    char  sis3316_ip_addr_string[32] ;
    strcpy(sis3316_ip_addr_string,"192.168.2.100") ; // SIS3316 IP address
    //int return_code ;
    strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
    return_code = vmei->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);
    
    vmei->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string);
    
    vmei->udp_reset_cmd();
    
    // open Vme Interface device
    return_code = vmei->vmeopen ();  // open Vme interface
    vmei->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
    
    printf("Ethernet interface opened and found number of devices %i with messages:\n%s\n", nof_found_devices, char_messages );
    
    
    //    now make our card
    sis3316card* sis3316 = new sis3316card(vmei, 0x0);
    
    //    attempt to read firmware id
    printf("attempting to read digitizer information (module ID register)\n");
    return_code = sis3316->vmei->vme_A32D32_read(SIS3316_MODID, &data );
    printf("read had return code %i, returned value %08x\n", return_code, data );

    
//    NOTE
//    granting of arbitration rights (or something like that) to eth interface is now done
//    automatically in the vmeopen() function of the ethB access class
//    it IS NECESSARY to do this before the interface is useful
//    
////    need to "grant" link access to other address space
////    ... or something...
////    first kill other requests
//    return_code = sis3316->vmei->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL,  0x80000000 );
//    printf("wrote to interface access arbitration control register with return code %i\n", return_code );
////    ARBITRATE
////    whatever that means
//    return_code = sis3316->vmei->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1);
//    printf("ARBITRATED with return code %i\n", return_code );
    
    return_code = sis3316->vmei->vme_A32D32_read(baseAddress+ 0x28, &data );
    printf( "read serial number %08x with return code %i\n", data, return_code );
    
    
////    set jumbo packets
//    return_code = ((sis3316_ethb*)sis3316->vmei)->set_UdpSocketEnableJumboFrame();
//    printf("set jumbo packet enable with return code %i\n", return_code );
//    set max packets
    return_code = ((sis3316_ethb*)sis3316->vmei)->set_UdpSocketReceiveNofPackagesPerRequest(32);
    printf("set 32 packets per request with return code %i\n", return_code );
    
//    no idea why the code below doesnt work
//    but the code above does!
    
//
//    sis3316_ethb* vmei = new sis3316_ethb();
//
//    int sockbufsize = 335544432 ; // 0x2000000
//    return_code = vmei->set_UdpSocketOptionBufSize(sockbufsize); //
//    if (return_code == -1) {
//        printf("Error: set_UdpSocketOptionSetBufSize\n");
//    }
//    return_code = vmei->set_UdpSocketBindMyOwnPort( ipAddress_ours );
//    if (return_code == -1) {
//        printf("Error: set_UdpSocketBindMyOwnPort\n");
//    }
//    return_code = vmei->set_UdpSocketSIS3316_IpAddress( ipAddress_digi );
//    if (return_code == -1) {
//        printf("Error: set_UdpSocketSIS3316_IpAddress\n");
//    }
//    
//    vmei->udp_reset_cmd();
//
//    vmei->vmeopen();
//    vmei->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
//   
//    printf("Ethernet interface opened and found number of devices %i with messages:\n%s\n", nof_found_devices, char_messages );
//    
////    unsigned int data;
//    
//    sis3316card* card = new sis3316card(vmei, baseAddress);
////    card->vmei = vmei;
////    card->baseaddress = baseAddress;
//    //vmei->vme_A32D32_write(baseAddress + 0x10/*SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL*/, 0x80000000);
//    // arbitrate
//    //vmei->vme_A32D32_write(baseAddress + 0x10/*SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL*/, 1);
//    return_code = card->vmei->vme_A32D32_read(SIS3316_MODID, &data );
////    return_code = card->vmei->vme_A32D32_read(baseAddress+0x4,&data);
//    
//    printf("attempt to read MODID register returns code %i with data 0x%x\n",return_code, data);
//    card->SetClockChoice(0,0);

  int expectedNumberOfWords = 0x400000;
//    int expectedNumberOfWords = 0x800000;
    
  TStopwatch ts;
  TStopwatch ts2;
    double totalTime = 0.0;
  ts.Start();
  unsigned int totalBytes = 0;
    addr = baseAddress+SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG;
    return_code = vmei->vme_A32D32_write(addr, 0x80000000);
    addr = baseAddress+SIS3316_FPGA_ADC1_MEM_BASE;
    return_code = vmei->vme_A32MBLT64FIFO_read( addr , buff, ((expectedNumberOfWords + 2) & 0xfffffC), &got_nof_32bit_words);
    //return_code = vmei->vme_A32_2ESST320FIFO_read( addr , buff, ((expectedNumberOfWords + 2) & 0xfffffC), &got_nof_32bit_words);
    if(return_code < 0) {
        printf("vme_A32_2ESST320_read: %d Address: %08x %08x\n",return_code, addr, 0x80000000);
        return return_code;
    }
    printf("Completed Test Read, got %i 32bit words.\n", got_nof_32bit_words);
    
    int numTrials = 100;
    for(int itrial = 0; itrial<numTrials; itrial++){
        printf("trial number %i of %i\n", itrial, numTrials );
        addr = baseAddress+SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG;
        return_code = vmei->vme_A32D32_write(addr, 0x80000000);
        if(return_code < 0) {
            printf("vme_A32D32_write: %d Address: %08x %08x\n", return_code, addr, 0x80000000);
            return return_code;
        }
        addr = baseAddress+SIS3316_FPGA_ADC1_MEM_BASE;
        ts2.Start();
        return_code = vmei->vme_A32MBLT64FIFO_read( addr , buff, ((expectedNumberOfWords + 2) & 0xfffffC), &got_nof_32bit_words);
        //return_code = vmei->vme_A32_2ESST320FIFO_read( addr , buff, ((expectedNumberOfWords + 2) & 0xfffffC), &got_nof_32bit_words);
        totalTime+=ts2.RealTime();
        if(return_code < 0) {
            printf("vme_A32_2ESST320_read: %d Address: %08x %08x\n",return_code, addr, 0x80000000);
            return return_code;
        }
        if(itrial==0) printf("Got Number of Words: 0x%x\n",got_nof_32bit_words);
        totalBytes+=got_nof_32bit_words*4;
    }
//    words -> MB is
//    nWords * 4 / 1024 / 1024
    printf("Read %u total bytes in %.2f real seconds or %.2f system seconds\n", totalBytes, ts.RealTime(), totalTime );
    printf("MB/s (real time) \t %.2f\n", totalBytes / ts.RealTime() / 1024 / 1024 );
    printf("MB/s (sys time) \t %.2f\n", totalBytes / totalTime / 1024 / 1024 );
    printf("MBytes per Second:%f %f\n",totalBytes/ts.RealTime()/0x400,totalBytes/totalTime/0x400);
    vmei->vmeclose();
    
    
    delete [] buff;
    return 0;
}

//sis3316_ethbernet_access_class

#ifndef _sis3316_ethbERNETB_ACCESS_CLASS_
#define _sis3316_ethbERNETB_ACCESS_CLASS_

#include "TObject.h"

// used to decide whether or not to use UDP "handshaking" features
#define VME_FPGA_VERSION_IS_0008_OR_HIGHER



// error codes
#define PROTOCOL_ERROR_CODE_WRONG_ACK						0x120
#define PROTOCOL_ERROR_CODE_WRONG_NOF_RECEVEID_BYTES		0x121
#define PROTOCOL_ERROR_CODE_WRONG_PACKET_IDENTIFIER			0x122
#define PROTOCOL_ERROR_CODE_WRONG_RECEIVED_PACKET_ORDER		0x123
#define PROTOCOL_ERROR_CODE_TIMEOUT							0x211



#define UDP_MAX_PACKETS_PER_REQUEST		    32            //  max. tramnsmit packets per Read Request to the PC
#define UDP_NORMAL_READ_PACKET_32bitSIZE    360           //  packet size = 1440 Bytes + 44/45 Bytes = 1484/1485 Bytes
#define UDP_JUMBO_READ_PACKET_32bitSIZE     2048          //  packet size = 8192 Bytes + 44/45 Bytes = 8236/8237 Bytes


/* nof_write_words: max. 0x100 (256 32-bit words = 1KBytes )  */
#define UDP_WRITE_PACKET_32bitSIZE		0x100

/* nof_read_words: max. 0x10000 (64k 32-bit words = 256KBytes )  */
#define UDP_MAX_NOF_READ_32bitWords    0x10000
/* nof_write_words: max. 0x100 (256 32-bit words = 1KBytes )  */
#define UDP_MAX_NOF_WRITE_32bitWords    0x100

#define SIS3316_UDP_PROT_CONFIGURATION                 					0x8			/* read/write; D32 */





#include <netinet/in.h>

#include "vme_interface_class.h"
typedef int8_t CHAR;
typedef uint8_t  UCHAR;
typedef uint16_t USHORT;

// on macosx arp -s 212.60.16.200 00:00:56:31:60:60 ifscope en4
#pragma once
class sis3316_ethb : public vme_interface_class
{
private:
    char digitizerIPaddress[32]; // IP address for the digitizer
    char pcIPaddress[32]; // IP address for the host PC (for p2p setup, leave "")
    char char_messages[128] ;
    int udp_socket_status;
    int udp_socket;
    unsigned int udp_port ;
    struct sockaddr_in SIS3316_sock_addr_in   ;
    struct sockaddr_in myPC_sock_addr   ;
    unsigned int recv_timeout_sec  ;
    unsigned int recv_timeout_usec  ;
    char udp_send_data[2048];
    char udp_recv_data[16384];
    
    unsigned int  jumbo_frame_enable;
    unsigned int  max_nof_read_lwords;
    unsigned int  max_nof_write_lwords;
    
    char  packet_identifier;
    
public:
    unsigned char  read_udp_register_receive_ack_retry_counter;
    unsigned char  read_sis3316_register_receive_ack_retry_counter;
    unsigned char  write_sis3316_register_receive_ack_retry_counter;
    unsigned char  read_sis3316_fifo_receive_ack_retry_counter;
    unsigned char  write_sis3316_fifo_receive_ack_retry_counter;
    
    
public:
    sis3316_ethb ();
    virtual ~sis3316_ethb(){}
    
    int vmeopen ( void );
    int vmeclose( void );
    int get_vmeopen_messages( CHAR* messages, uint32_t* nof_found_devices );
    
    
    // sets the digitizer IP address to be whatever is supplied
    void setDigitizerIPaddress( const char* digiIP );
    
    //    returns int, which is length of digitizer IP address that is copied into supplied char pointer
    //    digiIP is created in this function
    int getDigitizerIPaddress( char* digiIP );
    
    // sets the PC IP address to be whatever is supplied
    void setPCIPaddress( char* pcIP );
    
    //    returns int, which is length of PC IP address that is copied into supplied char pointer
    //    pcIP is created in this function
    int getPCIPaddress( char* pcIP );
    
    
    
    //int get_device_informations( USHORT* idVendor, USHORT* idProduct, USHORT* idSerNo, USHORT* idFirmwareVersion,  USHORT* idDriverVersion );
    int get_UdpSocketStatus( void );
    int get_UdpSocketPort(void );
    int set_UdpSocketOptionTimeout( unsigned int receive_timeout_sec, unsigned int receive_timeout_usec );
    int set_UdpSocketOptionBufSize( int sockbufsize );
    int set_UdpSocketBindToDevice( char* eth_device);
    int set_UdpSocketBindMyOwnPort( char* pc_ip_addr_string);
    int set_UdpSocketSIS3316_IpAddress( char* sis3316_ip_addr_string);
    
    unsigned int get_UdpSocketNofReadMaxLWordsPerRequest(void);
    unsigned int get_UdpSocketNofWriteMaxLWordsPerRequest(void);
    
    int set_UdpSocketReceiveNofPackagesPerRequest(unsigned int nofPacketsPerRequest);
    
    int get_UdpSocketJumboFrameStatus(void);
    int set_UdpSocketEnableJumboFrame(void);
    int set_UdpSocketDisableJumboFrame(void);
    
    
    int udp_reset_cmd( void);
    int udp_retransmit_cmd( int* receive_bytes, char* data_byte_ptr);
    int udp_register_read (uint32_t addr, uint32_t* data);
    int udp_sis3316_register_read ( unsigned int nof_read_registers, uint32_t* addr_ptr, uint32_t* data_ptr);
    int udp_register_write (uint32_t addr, uint32_t data);
    int udp_sis3316_register_write ( unsigned int nof_read_registers, uint32_t* addr_ptr, uint32_t* data_ptr);
    
    int udp_sub_sis3316_fifo_read ( unsigned int nof_read_words, uint32_t  addr, uint32_t* data_ptr);
    int udp_sis3316_fifo_read ( unsigned int nof_read_words, uint32_t  addr, uint32_t* data_ptr, uint32_t* got_nof_words );
    
    int udp_sub_sis3316_fifo_write ( unsigned int nof_write_words, uint32_t  addr, uint32_t* data_ptr);
    int udp_sis3316_fifo_write ( unsigned int nof_write_words, uint32_t addr, uint32_t* data_ptr, uint32_t* written_nof_words );
    
    int vme_A32D32_read (uint32_t addr, uint32_t* data);
    int vme_A32DMA_D32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32BLT32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32MBLT64_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2EVME_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2ESST160_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2ESST267_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2ESST320_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    
    int vme_A32DMA_D32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32BLT32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32MBLT64FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2EVMEFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2ESST160FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2ESST267FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    int vme_A32_2ESST320FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    
    int vme_A32D16_read( uint32_t addr, uint32_t* data ) {return -1;}
    int vme_A32D16_write( uint32_t addr, uint16_t data ) { return -1;}
    
    // wrapper function for whatever the "fastest", "highest-performance" FIFO read is available
    int vme_A32_FastestFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
    
    int vme_A32D32_write (uint32_t addr, uint32_t data);
    int vme_A32DMA_D32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
    int vme_A32BLT32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
    int vme_A32MBLT64_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
    int vme_A32DMA_D32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
    int vme_A32BLT32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
    int vme_A32MBLT64FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
    
    int vme_IRQ_Status_read( uint32_t* data ) ;
    
    ClassDef(sis3316_ethb, 0)
    
};

#endif

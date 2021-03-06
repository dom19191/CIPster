/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef CIPSTER_CIPSTER_API_H_
#define CIPSTER_CIPSTER_API_H_

#include <assert.h>


#include "typedefs.h"
#include "cip/ciptypes.h"
#include "cip/ciperror.h"
#include "cip/cipmessagerouter.h"
#include "byte_bufs.h"
#include "cipster_user_conf.h"



/**  @defgroup CIP_API CIPster User interface
 * @brief This is the public interface of the CIPster. It provides all function
 * needed to implement an EtherNet/IP enabled slave-device.
 */

/** @ingroup CIP_API
 * @brief Configure the data of the network interface of the device
 *
 *  This function setup the data of the network interface needed by CIPster.
 *  The multicast address is automatically calculated from he given data.
 *
 *  @param ip_address    the current IP address of the device
 *  @param subnet_mask  the subnet mask to be used
 *  @param gateway_address     the gateway address
 *  @return EIP_OK if the configuring worked otherwise EIP_ERROR
 */
EipStatus ConfigureNetworkInterface( const char* ip_address, const char* subnet_mask,
        const char* gateway_address );

/** @ingroup CIP_API
 * @brief Configure the MAC address of the device
 *
 *  @param mac_address  the hardware MAC address of the network interface
 */
void ConfigureMacAddress( const EipByte* mac_address );

/** @ingroup CIP_API
 * @brief Configure the domain name of the device
 * @param domain_name the domain name to be used
 */
void ConfigureDomainName( const char* domain_name );

/** @ingroup CIP_API
 * @brief Configure the host name of the device
 * @param host_name the host name to be used
 */
void ConfigureHostName( const char* host_name );

/** @ingroup CIP_API
 * @brief Set the serial number of the device's identity object.
 *
 * @param serial_number unique 32 bit number identifying the device
 */
void SetDeviceSerialNumber( EipUint32 serial_number );

/** @ingroup CIP_API
 * @brief Set the current status of the device.
 *
 * @param device_status the new status value
 */
void SetDeviceStatus( EipUint16 device_status );

/** @ingroup CIP_API
 * @brief Initialize and setup the CIP-stack
 *
 * @param unique_connection_id value passed to Connection_Manager_Init() to form
 * a "per boot" unique connection ID.
 */
void CipStackInit( EipUint16 unique_connection_id );

/** @ingroup CIP_API
 * @brief Shutdown of the CIP stack
 *
 * This will
 *   - close all open I/O connections,
 *   - close all open explicit connections, and
 *   - free all memory allocated by the stack.
 *
 * Memory allocated by the application will not be freed. This has to be done
 * by the application!
 */
void ShutdownCipStack();

/** @ingroup CIP_API
 * @brief Get a pointer to a CIP object with given class code
 *
 * @param class_id class ID of the object to retrieve
 * @return pointer to CIP Object
 *          0 if object is not present in the stack
 */
CipClass* GetCipClass( int class_id );


/** @ingroup CIP_API
 * @brief Register a CipClass into the CIP class registry.  This may only be
 *  done once for each unique class_id.
 *
 * @param aClass which CIP class to register.
 */
EipStatus RegisterCipClass( CipClass* aClass );


/** @ingroup CIP_API
 * @brief Serialize aDataType according to CIP encoding into aBuf
 *
 *  @param int aDataType the cip type to encode
 *  @param cip_data pointer to data value.
 *  @param aBuf where response should be written
 *  @return int - byte count writte into aBuf.
 *          -1 .. error
 */
int EncodeData( int aDataType, const void* cip_data, BufWriter& aBuf );

/** @ingroup CIP_API
 * @brief Retrieve the given data according to CIP encoding from the message
 * buffer.
 *
 * This function may be used in in own services for handling data from the
 * requester (e.g., setAttributeSingle).
 *  @param aDataType the CIP type to decode
 *  @param cip_data pointer to data value to written.
 *  @param aBuf where to get the data bytes from
 *  @return length of taken bytes
 *          -1 .. error
 */
int DecodeData( int aDataType, void* cip_data, BufReader& aBuf );

/** @ingroup CIP_API
 * @brief Create an instance of an assembly object
 *
 * @param aInstanceId  instance number of the assembly object to create
 * @param aBuffer      the data the assembly object should contain and its byte count.
 * @return CipInstance* - the instance of the created assembly object or NULL on error.
 *
 * Assembly Objects for Configuration Data:
 *
 * The CIP stack treats configuration assembly objects the same way as any other
 * assembly object.
 * In order to support a configuration assembly object it has to be created with
 * this function.
 * The notification on received configuration data is handled with the
 * IApp_after_receive function.
 */
CipInstance* CreateAssemblyInstance( int aInstanceId, BufWriter aBuffer );

class CipConn;

/** @ingroup CIP_API
 * @brief Function prototype for handling the closing of connections
 *
 * @param aConn The connection object which is closing the
 * connection
 */
typedef void (* ConnectionCloseFunction)( CipConn* aConn );

/** @ingroup CIP_API
 * @brief Function prototype for handling the timeout of connections
 *
 * @param aConn The connection object which connection timed out
 */
typedef void (* ConnectionTimeoutFunction)( CipConn* aConn );

/** @ingroup CIP_API
 * @brief Function prototype for sending data via a connection
 *
 * @param aConn The connection object which connection timed out
 *
 * @return EIP stack status
 */
typedef EipStatus (* ConnectionSendDataFunction)( CipConn* aConn );

/** @ingroup CIP_API
 * @brief Function prototype for receiving data via a connection
 *
 * @param aConn the connection object which connection timed out
 * @param aInput the payload of the CIP message with its length
 *
 * @return Stack status
 */
typedef EipStatus (* ConnectionReceiveDataFunction)( CipConn* aConn, BufReader aInput );

/** @ingroup CIP_API
 * @brief Configures the connection point for an exclusive owner connection.
 *
 * @param output_assembly_id ID of the O-to-T point to be used for this
 * connection
 * @param input_assembly_id ID of the T-to-O point to be used for this
 * connection
 * @param configuration_assembly_id ID of the configuration point to be used for
 * this connection
 * @return bool - true on success, else false if too many
 */
bool ConfigureExclusiveOwnerConnectionPoint(
        int output_assembly_id,
        int input_assembly_id,
        int configuration_assembly_id );

/** @ingroup CIP_API
 * @brief Configures the connection point for an input only connection.
 *
 * @param output_assembly_id ID of the O-to-T point to be used for this
 * connection
 * @param input_assembly_id ID of the T-to-O point to be used for this
 * connection
 * @param configuration_assembly_id ID of the configuration point to be used for
 * this connection
 * @return bool - true on success, else false if too many
 */
bool ConfigureInputOnlyConnectionPoint(
        int output_assembly_id,
        int input_assembly_id,
        int configuration_assembly_id );

/** \ingroup CIP_API
 * \brief Configures the connection point for a listen only connection.
 *
 * @param connection_number The number of the input only connection. The
 *        enumeration starts with 0. Has to be smaller than
 *        CIPSTER_CIP_NUM_LISTEN_ONLY_CONNS.
 * @param output_assembly_id ID of the O-to-T point to be used for this
 * connection
 * @param input_assembly_id ID of the T-to-O point to be used for this
 * connection
 * @param configuration_assembly_id ID of the configuration point to be used for
 * this connection
 * @return bool - true on success, else false if too many
 */
bool ConfigureListenOnlyConnectionPoint(
        int output_assembly_id,
        int input_assembly_id,
        int configuration_assembly_id );

/**
 * Function HandleReceivedExplicitTcpData
 * notifies the encapsulation layer that an explicit message has been
 * received via TCP.
 *
 * @param socket the BSD socket from which data is received.
 * @param aCommand is the buffer that contains the received data.
 * @param aReply is the buffer that should be used for the reply.
 * @return int - byte count of reply that needs to be sent back
 */
int HandleReceivedExplictTcpData( int socket, BufReader aCommand, BufWriter aReply );

/**
 * Function HandleReceivedExplicitUdpData
 * notifies the encapsulation layer that an explicit message has been
 * received via UDP.
 *
 * @param socket BSD socket from which data is received.
 * @param from_address remote address from which the data is received.
 * @param aCommand received data buffer pointing just past the encapsulation header and its length.
 * @param aReply where to put reply and tells its maximum length.
 * @param isUnicast true if unicast, else false.
 * @return int - byte count of reply that need to be sent back
 */
int HandleReceivedExplictUdpData( int socket, const sockaddr_in* from_address,
        BufReader aCommand,  BufWriter aReply, bool isUnicast );

/**
 * Function HandleReceivedConnectedData
 * notifies the connection manager that data for a connection has been
 * received.
 *
 * This function should be invoked by the network layer.
 * @param from_address address from which the data has been received. Only
 *           data from the connections originator may be accepted. Avoids
 *           connection hijacking
 * @param aCommand received data buffer pointing just past the
 *   encapsulation header and a byte count remaining in frame.
 * @param aReply where to put the reply and tells its maximum length.
 * @return EipStatus
 */
EipStatus HandleReceivedConnectedData( const sockaddr_in* from_address, BufReader aCommand );

/** @ingroup CIP_API
 * @brief Check if any of the connection timers (TransmissionTrigger or
 * WatchdogTimeout) have timed out.
 *
 * If the a timeout occurs the function performs the necessary action. This
 * function should be called periodically once every CIPSTER_TIMER_TICK
 * milliseconds.
 *
 * @return EIP_OK on success
 */
EipStatus ManageConnections();

/** @ingroup CIP_API
 * @brief Trigger the production of an application triggered connection.
 *
 * This will issue the production of the specified connection at the next
 * possible occasion. Depending on the values for the RPI and the production
 * inhibit timer. The application is informed via the
 * bool BeforeAssemblyDataSend( CipInstance* aInstance )
 * callback function when the production will happen. This function should only
 * be invoked from void HandleApplication().
 *
 * The connection can only be triggered if the application is established and it
 * is of application triggered type.
 *
 * @param output_assembly_id the output assembly connection point of the
 * connection
 * @param input_assembly_id the input assembly connection point of the
 * connection
 * @return EIP_OK on success
 */
EipStatus TriggerConnections( int output_assembly_id, int input_assembly_id );

/** @ingroup CIP_API
 * @brief Inform the encapsulation layer that the remote host has closed the
 * connection.
 *
 * According to the specifications that will clean up and close the session in
 * the encapsulation layer.
 * @param socket_handle the handler to the socket of the closed connection
 */
void CloseSession( int socket );

/**  @defgroup CIP_CALLBACK_API Callback Functions Demanded by CIPster
 * @ingroup CIP_API
 *
 * @brief These functions have to implemented in order to give the CIPster a
 * method to inform the application on certain state changes.
 */

/** @ingroup CIP_CALLBACK_API
 * @brief Callback for the application initialization
 *
 * This function will be called by the CIP stack after it has finished its
 * initialization. In this function the user can setup all CIP objects she
 * likes to have.
 *
 * This function is provided for convenience reasons. After the void
 * CipStackInit(void)
 * function has finished it is okay to also generate your CIP objects.
 *  return status EIP_ERROR .. error
 *                EIP_OK ... successful finish
 */
EipStatus ApplicationInitialization();

/** @ingroup CIP_CALLBACK_API
 * @brief Allow the device specific application to perform its execution
 *
 * This function will be executed by the stack at the beginning of each
 * execution of EIP_STATUS ManageConnections(void). It allows to implement
 * device specific application functions. Execution within this function should
 * be short.
 */
void HandleApplication();

/** @ingroup CIP_CALLBACK_API
 * @brief Inform the application on changes occurred for a connection
 *
 * @param output_assembly_id the output assembly connection point of the
 * connection
 * @param input_assembly_id the input assembly connection point of the
 * connection
 * @param io_connection_event information on the change occurred
 */
void CheckIoConnectionEvent( int output_assembly_id, int input_assembly_id,
        IoConnectionEvent io_connection_event );

/** @ingroup CIP_CALLBACK_API
 * @brief Call back function to inform application on received data for an
 * assembly object.
 *
 * This function has to be implemented by the user of the CIP-stack.
 * @param instance pointer to the assembly object data was received for
 * @return Information if the data could be processed
 *     - EIP_OK the received data was ok
 *     - EIP_ERROR the received data was wrong (especially needed for
 * configuration data assembly objects)
 *
 * Assembly Objects for Configuration Data:
 * The CIP-stack uses this function to inform on received configuration data.
 * The length of the data is already checked within the stack. Therefore the
 * user only has to check if the data is valid.
 */
EipStatus AfterAssemblyDataReceived( CipInstance* instance );

/** @ingroup CIP_CALLBACK_API
 * @brief Inform the application that the data of an assembly
 * object will be sent.
 *
 * Within this function the user can update the data of the assembly object
 * before it gets sent. The application can inform the stack if data has
 * changed.
 * @param aInstance is the assembly instance that should send data.
 * @return data has changed:
 *          - true assembly data has changed
 *          - false assembly data has not changed
 */
bool BeforeAssemblyDataSend( CipInstance* aInstance );

/** @ingroup CIP_CALLBACK_API
 * @brief Emulate as close a possible a power cycle of the device
 *
 * @return if the service is supported the function will not return.
 *     EIP_ERROR if this service is not supported
 */
EipStatus ResetDevice();

/** @ingroup CIP_CALLBACK_API
 * @brief Reset the device to the initial configuration and emulate as close as
 * possible a power cycle of the device
 * @param also_reset_comm_parameters when true means reset everything including
 *   communications parameters, else all but comm params.
 *
 * @return if the service is supported the function will not return.
 *     EIP_ERROR if this service is not supported
 *
 * @see CIP Identity Service 5: Reset
 */
EipStatus ResetDeviceToInitialConfiguration( bool also_reset_comm_parameters );

#if defined(__linux__) || defined(__WIN32)

#include <stdlib.h>

static inline void* CipCalloc( unsigned pa_nNumberOfElements, unsigned pa_nSizeOfElement )
{
    return calloc( pa_nNumberOfElements, pa_nSizeOfElement );
}


static inline void CipFree( void* pa_poData )
{
    free( pa_poData );
}


#else

/** @ingroup CIP_CALLBACK_API
 * @brief Allocate memory for the CIP stack
 *
 * emulate the common c-library function calloc
 * In CIPster allocation only happens on application startup and on
 * class/instance creation and configuration not on during operation
 * (processing messages).
 * @param number_of_elements number of elements to allocate
 * @param size_of_element size in bytes of one element
 * @return pointer to the allocated memory, 0 on error
 */
void* CipCalloc( unsigned number_of_elements, unsigned size_of_element );

/** @ingroup CIP_CALLBACK_API
 * @brief Free memory allocated by the CIPster
 *
 * emulate the common c-library function free
 * @param pa_poData pointer to the allocated memory
 */
void CipFree( void* data );

#endif


/** @ingroup CIP_CALLBACK_API
 * @brief Inform the application that the Run/Idle State has been changed
 * by the originator.
 *
 * @param run_idle_value the current value of the run/idle flag according to CIP
 * spec Vol 1 3-6.5
 */
void RunIdleChanged( EipUint32 run_idle_value );

/** @ingroup CIP_CALLBACK_API
 * @brief create a producing or consuming UDP socket
 *
 * @param communication_direction PRODCUER or CONSUMER
 * @param socket_data pointer to the address holding structure
 *     Attention: For producing point-to-point connection the
 *     *pa_pstAddr->sin_addr.s_addr member is set to 0 by CIPster. The network
 *     layer of the application has to set the correct address of the
 *     originator.
 *     Attention: For consuming connection the network layer has to set the
 * pa_pstAddr->sin_addr.s_addr to the correct address of the originator.
 * FIXME add an additional parameter that can be used by the CIP stack to
 * request the originators sockaddr_in data.
 * @return socket identifier on success
 *         -1 on error
 */
int CreateUdpSocket( UdpCommuncationDirection communication_direction,
        struct sockaddr_in* socket_data );

/** @ingroup CIP_CALLBACK_API
 * @brief create a producing or consuming UDP socket
 *
 * @param socket_data pointer to the "send to" address
 * @param socket_handle socket descriptor to send on
 * @param aOutput the data to send and its length
 * @return  EIP_SUCCESS on success
 */
EipStatus SendUdpData( struct sockaddr_in* socket_data, int socket, BufReader aOutput );

/** @ingroup CIP_CALLBACK_API
 * @brief Close the given socket and clean up the stack
 *
 * @param socket_handle socket descriptor to close
 */
void CloseSocket( int socket );


void IApp_CloseSocket_udp( int socket_handle );
void IApp_CloseSocket_tcp( int socket_handle );


/** @mainpage CIPster - Open Source EtherNet/IP(TM) Communication Stack
 * Documentation
 *
 * EtherNet/IP stack for adapter devices (connection target); supports multiple
 * I/O and explicit connections; includes features and objects required by the
 * CIP specification to enable devices to comply with ODVA's conformance/
 * interoperability tests.
 *
 * @section intro_sec Introduction
 *
 * This is the introduction.
 *
 * @section install_sec Building
 * How to compile, install and run CIPster on a specific platform.
 *
 * @subsection build_req_sec Requirements
 * CIPster has been developed to be highly portable. The default version targets
 * PCs with a POSIX operating system and a BSD-socket network interface. To
 * test this version we recommend a Linux PC or Windows with Cygwin installed.
 *  You will need to have the following installed:
 *   - gcc, make, binutils, etc.
 *
 * for normal building. These should be installed on most Linux installations
 * and are part of the development packages of Cygwin.
 *
 * For the development itself we recommend the use of Eclipse with the CDT
 * plugin. For your convenience CIPster already comes with an Eclipse project
 * file. This allows to just import the CIPster source tree into Eclipse.
 *
 * @subsection compile_pcs_sec Compile for PCs
 *   -# Directly in the shell
 *       -# Go into the bin/pc directory
 *       -# Invoke make
 *       -# For invoking opener type:\n
 *          ./opener ipaddress subnetmask gateway domainname hostaddress
 * macaddress\n
 *          e.g., ./opener 192.168.0.2 255.255.255.0 192.168.0.1 test.com
 * testdevice 00 15 C5 BF D0 87
 *   -# Within Eclipse
 *       -# Import the project
 *       -# Go to the bin/pc folder in the make targets view
 *       -# Choose all from the make targets
 *       -# The resulting executable will be in the directory
 *           ./bin/pc
 *       -# The command line parameters can be set in the run configuration
 * dialog of Eclipse
 *
 * @section further_reading_sec Further Topics
 *   - @ref porting
 *   - @ref extending
 *   - @ref license
 *
 * @page porting Porting CIPster
 * @section gen_config_section General Stack Configuration
 * The general stack properties have to be defined prior to building your
 * production. This is done by providing a file called cipster_user_conf.h. An
 * example file can be found in the src/ports/platform-pc directory. The
 * documentation of the example file for the necessary configuration options:
 * cipster_user_conf.h
 *
 * @copydoc cipster_user_conf.h
 *
 * @section startup_sec Startup Sequence
 * During startup of your EtherNet/IP(TM) device the following steps have to be
 * performed:
 *   -# Configure the network properties:\n
 *       With the following functions the network interface of CIPster is
 *       configured:
 *        - EIP_STATUS ConfigureNetworkInterface(const char *ip_address,
 *        const char *subnet_mask, const char *gateway_address)
 *        - void ConfigureMACAddress(const EIP_UINT8 *mac_address)
 *        - void ConfigureDomainName(const char *domain_name)
 *        - void ConfigureHostName(const char *host_name)
 *        .
 *       Depending on your platform these data can come from a configuration
 *       file or from operating system functions. If these values should be
 *       setable remotely via explicit messages the SetAttributeSingle functions
 *       of the EtherNetLink and the TCPIPInterface object have to be adapted.
 *   -# Set the device's serial number\n
 *      According to the CIP specification a device vendor has to ensure that
 *      each of its devices has a unique 32Bit device id. You can set it with
 *      the function:
 *       - void setDeviceSerialNumber(EIP_UINT32 serial_number)
 *   -# Initialize CIPster: \n
 *      With the function CipStackInit(EIP_UINT16 unique_connection_id) the
 *      internal data structures of opener are correctly setup. After this
 *      step own CIP objects and Assembly objects instances may be created. For
 *      your convenience we provide the call-back function
 *      ApplicationInitialization. This call back function is called when the
 * stack is ready to receive application specific CIP objects.
 *   -# Create Application Specific CIP Objects:\n
 *      Within the call-back function ApplicationInitialization(void) or
 *      after CipStackInit(void) has finished you may create and configure any
 *      CIP object or Assembly object instances. See the module @ref CIP_API
 *      for available functions. Currently no functions are available to
 *      remove any created objects or instances. This is planned
 *      for future versions.
 *   -# Setup the listening TCP and UDP port:\n
 *      THE ETHERNET/IP SPECIFICATION demands from devices to listen to TCP
 *      connections and UDP datagrams on the port AF12hex for explicit messages.
 *      Therefore before going into normal operation you need to configure your
 *      network library so that TCP and UDP messages on this port will be
 *      received and can be hand over to the Ethernet encapsulation layer.
 *
 * @section normal_op_sec Normal Operation
 * During normal operation the following tasks have to be done by the platform
 * specific code:
 *   - Establish connections requested on TCP port AF12hex
 *   - Receive explicit message data on connected TCP sockets and the UPD socket
 *     for port AF12hex. The received data has to be handed over to Ethernet
 *     encapsulation layer with the functions: \n
 *     int HandleReceivedExplictTcpData( int socket, CipBufUnmutable aCommand, BufWriter aReply ),\n
 *     int HandleReceivedExplictUDPData(int socket_handle, struct sockaddr_in
 * *from_address, EIP_UINT8* buffer, unsigned buffer_length, int
 * *number_of_remaining_bytes).\n
 *     Depending if the data has been received from a TCP or from a UDP socket.
 *     As a result of this function a response may have to be sent. The data to
 *     be sent is in the given buffer pa_buf.
 *   - Create UDP sending and receiving sockets for implicit connected
 * messages\n
 *     CIPster will use to call-back function int CreateUdpSocket(
 *     UdpCommuncationDirection connection_direction,
 *     struct sockaddr_in *pa_pstAddr)
 *     for informing the platform specific code that a new connection is
 *     established and new sockets are necessary
 *   - Receive implicit connected data on a receiving UDP socket\n
 *     The received data has to be hand over to the Connection Manager Object
 *     with the function EipStatus HandleReceivedConnectedData( const sockaddr_in* from_address, BufReader aCommand );
 *   - Close UDP and TCP sockets:
 *      -# Requested by CIPster through the call back function: void
 * CloseSocket(int socket_handle)
 *      -# For TCP connection when the peer closed the connection CIPster needs
 *         to be informed to clean up internal data structures. This is done
 * with
 *         the function void CloseSession(int socket_handle).
 *      .
 *   - Cyclically update the connection status:\n
 *     In order that CIPster can determine when to produce new data on
 *     connections or that a connection timed out every @ref CIPSTER_TIMER_TICK
 * milliseconds the
 *     function EIP_STATUS ManageConnections(void) has to be called.
 *
 * @section callback_funcs_sec Callback Functions
 * In order to make CIPster more platform independent and in order to inform the
 * application on certain state changes and actions within the stack a set of
 * call-back functions is provided. These call-back functions are declared in
 * the file cipster_api.h and have to be implemented by the application specific
 * code. An overview and explanation of CIPster's call-back API may be found in
 * the module @ref CIP_CALLBACK_API.
 *
 * @page extending Extending CIPster
 * CIPster provides an API for adding own CIP objects and instances with
 * specific services and attributes. Therefore CIPster can be easily adapted to
 * support different device profiles and specific CIP objects needed for your
 * device. The functions to be used are:
 *   - S_CIP_Class *CreateCIPClass(EIP_UINT32 class_id, int
 * number_of_class_attributes, EIP_UINT32 class_get_attribute_all_mask, int
 * number_of_class_services, int number_of_instance_attributes, EIP_UINT32
 * instance_get_attribute_all_mask, int number_of_instance_services, int
 * number_of_instances, char *class_name, EIP_UINT16 revision);
 *   - S_CIP_Instance *AddCIPInstances(S_CIP_Class *cip_object, int
 * number_of_instances);
 *   - S_CIP_Instance *AddCIPInstance(S_CIP_Class * cip_class, EIP_UINT32
 * instance_id);
 *   - void InsertAttribute(S_CIP_Instance *instance, EIP_UINT16
 * attribute_number, EIP_UINT8 cip_type, void* data);
 *   - void InsertService(S_CIP_Class *class, EIP_UINT8 service_number,
 * CipServiceFunction service_function, char *service_name);
 *
 * @page license CIPster Open Source License
 * The CIPster Open Source License is an adapted BSD style license. The
 * adaptations include the use of the term EtherNet/IP(TM) and the necessary
 * guarding conditions for using CIPster in own products. For this please look
 * in license text as shown below:
 *
 * @include "license.txt"
 *
 */


#endif //CIPSTER_CIPSTER_API_H_

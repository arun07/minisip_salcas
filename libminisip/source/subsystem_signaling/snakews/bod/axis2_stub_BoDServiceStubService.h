

        /**
        * axis2_stub_BoDServiceStubService.h
        *
        * This file was auto-generated from WSDL for "BoDServiceStubService|http://esb.bod.services.hdviper.i2cat.net/" service
        * by the Apache Axis2/Java version: 1.4.1  Built on : Aug 13, 2008 (05:03:35 LKT)
        */

        #include <stdio.h>
        #include <axiom.h>
        #include <axutil_utils.h>
        #include <axiom_soap.h>
        #include <axis2_client.h>
        #include <axis2_stub.h>

       
         #include "adb_reservationStatusChanged1.h"
        
         #include "reservationStatusChangedResponse2.h"
        

	#ifdef __cplusplus
	extern "C" {
	#endif

        /***************** function prototypes - for header file *************/
        /**
         * axis2_stub_create_BoDServiceStubService
         * Create and return the stub with services populated
         * @param env Environment ( mandatory)
         * @param client_home Axis2/C home ( mandatory )
         * @param endpoint_uri Service endpoint uri( optional ) - if NULL default picked from WSDL used
         * @return Newly created stub object
         */
        axis2_stub_t*
        axis2_stub_create_BoDServiceStubService(const axutil_env_t *env,
                                        axis2_char_t *client_home,
                                        axis2_char_t *endpoint_uri);
        /**
         * axis2_stub_populate_services_for_BoDServiceStubService
         * populate the svc in stub with the service and operations
         * @param stub The stub
         * @param env environment ( mandatory)
         */
        void axis2_stub_populate_services_for_BoDServiceStubService( axis2_stub_t *stub, const axutil_env_t *env);
        /**
         * axis2_stub_get_endpoint_uri_of_BoDServiceStubService
         * Return the endpoint URI picked from WSDL
         * @param env environment ( mandatory)
         * @return The endpoint picked from WSDL
         */
        axis2_char_t *
        axis2_stub_get_endpoint_uri_of_BoDServiceStubService(const axutil_env_t *env);

        
            /**
             * Auto generated function declaration
             * for "reservationStatusChanged|http://esb.bod.services.hdviper.i2cat.net/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             *
             * @param _reservationStatusChanged
             *
             * @return adb_reservationStatusChangedResponse2_t*
             */


            adb_reservationStatusChangedResponse2_t* 
            axis2_stub_op_BoDServiceStubService_reservationStatusChanged( axis2_stub_t *stub, const axutil_env_t *env,
                                                        adb_reservationStatusChanged1_t* _reservationStatusChanged);
          
        /**
         * Auto generated function for asynchronous invocations
         * for "reservationStatusChanged|http://esb.bod.services.hdviper.i2cat.net/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         
         * @param _reservationStatusChanged
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */

        

        void axis2_stub_start_op_BoDServiceStubService_reservationStatusChanged( axis2_stub_t *stub, const axutil_env_t *env,
                                                    adb_reservationStatusChanged1_t* _reservationStatusChanged,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_reservationStatusChangedResponse2_t* _reservationStatusChangedResponse, void *data) ,
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

          

     /**
      * function to free any soap input headers 
      * @param env environment ( mandatory)
      */
     

     /**
      * function to free any soap output headers 
      * @param env environment ( mandatory)
      */
     

	#ifdef __cplusplus
	}
	#endif
   


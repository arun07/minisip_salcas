

        /**
         * adb_reservationStatusChanged.c
         *
         * This file was auto-generated from WSDL
         * by the Apache Axis2/C version: SNAPSHOT  Built on : Mar 10, 2008 (08:35:52 GMT+00:00)
         */

        #include "adb_reservationStatusChanged.h"
        
                /*
                 * This type was generated from the piece of schema that had
                 * name = reservationStatusChanged
                 * Namespace URI = http://esb.bod.services.hdviper.i2cat.net/
                 * Namespace Prefix = ns1
                 */
           


        struct adb_reservationStatusChanged
        {
            adb_sipAddress_t* property_userAddress;

                
                axis2_bool_t is_valid_userAddress;


            
        };


       /************************* Private Function prototypes ********************************/
        

                axis2_status_t AXIS2_CALL
                adb_reservationStatusChanged_set_userAddress_nil(
                        adb_reservationStatusChanged_t* _reservationStatusChanged,
                        const axutil_env_t *env);
            


       /************************* Function Implmentations ********************************/
        adb_reservationStatusChanged_t* AXIS2_CALL
        adb_reservationStatusChanged_create(
            const axutil_env_t *env)
        {
            adb_reservationStatusChanged_t *_reservationStatusChanged = NULL;
            
            AXIS2_ENV_CHECK(env, NULL);

            _reservationStatusChanged = (adb_reservationStatusChanged_t *) AXIS2_MALLOC(env->
                allocator, sizeof(adb_reservationStatusChanged_t));

            if(NULL == _reservationStatusChanged)
            {
                AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
                return NULL;
            }

            memset(_reservationStatusChanged, 0, sizeof(adb_reservationStatusChanged_t));

            _reservationStatusChanged->property_userAddress  = NULL;
                  _reservationStatusChanged->is_valid_userAddress  = AXIS2_FALSE;
            

            return _reservationStatusChanged;
        }

        axis2_status_t AXIS2_CALL
        adb_reservationStatusChanged_free (
                adb_reservationStatusChanged_t* _reservationStatusChanged,
                const axutil_env_t *env)
        {
            

            AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
            AXIS2_PARAM_CHECK(env->error, _reservationStatusChanged, AXIS2_FAILURE);

            adb_reservationStatusChanged_reset_userAddress(_reservationStatusChanged, env);
            

            if(_reservationStatusChanged)
            {
                AXIS2_FREE(env->allocator, _reservationStatusChanged);
                _reservationStatusChanged = NULL;
            }
            return AXIS2_SUCCESS;
        }


        

        axis2_status_t AXIS2_CALL
        adb_reservationStatusChanged_deserialize(
                adb_reservationStatusChanged_t* _reservationStatusChanged,
                const axutil_env_t *env,
                axiom_node_t **dp_parent,
                axis2_bool_t *dp_is_early_node_valid,
                axis2_bool_t dont_care_minoccurs)
        {
          axiom_node_t *parent = *dp_parent;
          
          axis2_status_t status = AXIS2_SUCCESS;
          
              void *element = NULL;
           
             axis2_char_t* text_value = NULL;
             axutil_qname_t *qname = NULL;
          
            axutil_qname_t *element_qname = NULL; 
            
               axiom_node_t *first_node = NULL;
               axis2_bool_t is_early_node_valid = AXIS2_TRUE;
               axiom_node_t *current_node = NULL;
               axiom_element_t *current_element = NULL;
            
            AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
            AXIS2_PARAM_CHECK(env->error, _reservationStatusChanged, AXIS2_FAILURE);

            
              
              while(parent && axiom_node_get_node_type(parent, env) != AXIOM_ELEMENT)
              {
                  parent = axiom_node_get_next_sibling(parent, env);
              }
              if (NULL == parent)
              {
                /* This should be checked before everything */
                AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, 
                            "Failed in building adb object for reservationStatusChanged : "
                            "NULL elemenet can not be passed to deserialize");
                return AXIS2_FAILURE;
              }
              
                      
                      first_node = axiom_node_get_first_child(parent, env);
                      
                    

                     
                     /*
                      * building userAddress element
                      */
                     
                     
                     
                                   current_node = first_node;
                                   is_early_node_valid = AXIS2_FALSE;
                                   
                                   
                                    while(current_node && axiom_node_get_node_type(current_node, env) != AXIOM_ELEMENT)
                                    {
                                        current_node = axiom_node_get_next_sibling(current_node, env);
                                    }
                                    if(current_node != NULL)
                                    {
                                        current_element = (axiom_element_t *)axiom_node_get_data_element(current_node, env);
                                        qname = axiom_element_get_qname(current_element, env, current_node);
                                    }
                                   
                                 element_qname = axutil_qname_create(env, "userAddress", NULL, NULL);
                                 

                           if (adb_sipAddress_is_particle() ||  
                                (current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("userAddress", axiom_element_get_localname(current_element, env)))))
                           {
                              if( current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("userAddress", axiom_element_get_localname(current_element, env))))
                              {
                                is_early_node_valid = AXIS2_TRUE;
                              }
                              
                                 
                                      element = (void*)adb_sipAddress_create(env);

                                      status =  adb_sipAddress_deserialize((adb_sipAddress_t*)element,
                                                                            env, &current_node, &is_early_node_valid, AXIS2_FALSE);
                                      if(AXIS2_FAILURE == status)
                                      {
                                          AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in building adb object for element userAddress");
                                      }
                                      else
                                      {
                                          status = adb_reservationStatusChanged_set_userAddress(_reservationStatusChanged, env,
                                                                   (adb_sipAddress_t*)element);
                                      }
                                    
                                 if(AXIS2_FAILURE ==  status)
                                 {
                                     AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in setting the value for userAddress ");
                                     if(element_qname)
                                     {
                                         axutil_qname_free(element_qname, env);
                                     }
                                     return AXIS2_FAILURE;
                                 }
                              }
                           
                  if(element_qname)
                  {
                     axutil_qname_free(element_qname, env);
                     element_qname = NULL;
                  }
                 
          return status;
       }

          axis2_bool_t AXIS2_CALL
          adb_reservationStatusChanged_is_particle()
          {
            
                 return AXIS2_FALSE;
              
          }


          void AXIS2_CALL
          adb_reservationStatusChanged_declare_parent_namespaces(
                    adb_reservationStatusChanged_t* _reservationStatusChanged,
                    const axutil_env_t *env, axiom_element_t *parent_element,
                    axutil_hash_t *namespaces, int *next_ns_index)
          {
            
                  /* Here this is an empty function, Nothing to declare */
                 
          }

        
        
        axiom_node_t* AXIS2_CALL
        adb_reservationStatusChanged_serialize(
                adb_reservationStatusChanged_t* _reservationStatusChanged,
                const axutil_env_t *env, axiom_node_t *parent, axiom_element_t *parent_element, int parent_tag_closed, axutil_hash_t *namespaces, int *next_ns_index)
        {
            
            
             axis2_char_t *string_to_stream;
            
         
         axiom_node_t *current_node = NULL;
         int tag_closed = 0;

         
         
                axiom_namespace_t *ns1 = NULL;

                axis2_char_t *qname_uri = NULL;
                axis2_char_t *qname_prefix = NULL;
                axis2_char_t *p_prefix = NULL;
                axis2_bool_t ns_already_defined;
            
                    axis2_char_t text_value_1[64];
                    
               axis2_char_t *start_input_str = NULL;
               axis2_char_t *end_input_str = NULL;
               unsigned int start_input_str_len = 0;
               unsigned int end_input_str_len = 0;
            
            
               axiom_data_source_t *data_source = NULL;
               axutil_stream_t *stream = NULL;

            

            AXIS2_ENV_CHECK(env, NULL);
            AXIS2_PARAM_CHECK(env->error, _reservationStatusChanged, NULL);
            
            
                    current_node = parent;
                    data_source = (axiom_data_source_t *)axiom_node_get_data_element(current_node, env);
                    if (!data_source)
                        return NULL;
                    stream = axiom_data_source_get_stream(data_source, env); /* assume parent is of type data source */
                    if (!stream)
                        return NULL;
                  
            if(!parent_tag_closed)
            {
            
              string_to_stream = ">"; 
              axutil_stream_write(stream, env, string_to_stream, axutil_strlen(string_to_stream));
              tag_closed = 1;
            
            }
            
                       p_prefix = NULL;
                      

                   if (!_reservationStatusChanged->is_valid_userAddress)
                   {
                      
                           /* no need to complain for minoccurs=0 element */
                            
                          
                   }
                   else
                   {
                     start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (4 + axutil_strlen(p_prefix) + 
                                  axutil_strlen("userAddress"))); 
                                 
                                 /* axutil_strlen("<:>") + 1 = 4 */
                     end_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (5 + axutil_strlen(p_prefix) + axutil_strlen("userAddress")));
                                  /* axutil_strlen("</:>") + 1 = 5 */
                                  
                     

                   
                   
                     
                     /*
                      * parsing userAddress element
                      */

                    
                    
                            sprintf(start_input_str, "<%s%suserAddress",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":""); 
                            
                        start_input_str_len = axutil_strlen(start_input_str);
                        sprintf(end_input_str, "</%s%suserAddress>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                        end_input_str_len = axutil_strlen(end_input_str);
                     
                            if(!adb_sipAddress_is_particle())
                            {
                                axutil_stream_write(stream, env, start_input_str, start_input_str_len);
                            }
                            
                            adb_sipAddress_serialize(_reservationStatusChanged->property_userAddress, 
                                                                                 env, current_node, parent_element,
                                                                                 adb_sipAddress_is_particle() || AXIS2_FALSE, namespaces, next_ns_index);
                            
                            if(!adb_sipAddress_is_particle())
                            {
                                axutil_stream_write(stream, env, end_input_str, end_input_str_len);
                            }
                            
                     
                     AXIS2_FREE(env->allocator,start_input_str);
                     AXIS2_FREE(env->allocator,end_input_str);
                 } 

                 

            return parent;
        }


        

            /**
             * getter for userAddress.
             */
            adb_sipAddress_t* AXIS2_CALL
            adb_reservationStatusChanged_get_userAddress(
                    adb_reservationStatusChanged_t* _reservationStatusChanged,
                    const axutil_env_t *env)
             {
                
                    AXIS2_ENV_CHECK(env, NULL);
                    AXIS2_PARAM_CHECK(env->error, _reservationStatusChanged, NULL);
                  

                return _reservationStatusChanged->property_userAddress;
             }

            /**
             * setter for userAddress
             */
            axis2_status_t AXIS2_CALL
            adb_reservationStatusChanged_set_userAddress(
                    adb_reservationStatusChanged_t* _reservationStatusChanged,
                    const axutil_env_t *env,
                    adb_sipAddress_t*  arg_userAddress)
             {
                

                AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
                AXIS2_PARAM_CHECK(env->error, _reservationStatusChanged, AXIS2_FAILURE);
                
                if(_reservationStatusChanged->is_valid_userAddress &&
                        arg_userAddress == _reservationStatusChanged->property_userAddress)
                {
                    
                    return AXIS2_SUCCESS; 
                }

                adb_reservationStatusChanged_reset_userAddress(_reservationStatusChanged, env);

                
                if(NULL == arg_userAddress)
                {
                    /* We are already done */
                    return AXIS2_SUCCESS;
                }
                _reservationStatusChanged->property_userAddress = arg_userAddress;
                        _reservationStatusChanged->is_valid_userAddress = AXIS2_TRUE;
                    
                return AXIS2_SUCCESS;
             }

             

           /**
            * resetter for userAddress
            */
           axis2_status_t AXIS2_CALL
           adb_reservationStatusChanged_reset_userAddress(
                   adb_reservationStatusChanged_t* _reservationStatusChanged,
                   const axutil_env_t *env)
           {
               int i = 0;
               int count = 0;
               void *element = NULL;

               AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
               AXIS2_PARAM_CHECK(env->error, _reservationStatusChanged, AXIS2_FAILURE);
               

               
            
                
                if(_reservationStatusChanged->property_userAddress != NULL)
                {
                   
                   
                        adb_sipAddress_free(_reservationStatusChanged->property_userAddress, env);
                     _reservationStatusChanged->property_userAddress = NULL;
                }
            
                
                
                _reservationStatusChanged->is_valid_userAddress = AXIS2_FALSE; 
               return AXIS2_SUCCESS;
           }

           /**
            * Check whether userAddress is nill
            */
           axis2_bool_t AXIS2_CALL
           adb_reservationStatusChanged_is_userAddress_nil(
                   adb_reservationStatusChanged_t* _reservationStatusChanged,
                   const axutil_env_t *env)
           {
               AXIS2_ENV_CHECK(env, AXIS2_TRUE);
               AXIS2_PARAM_CHECK(env->error, _reservationStatusChanged, AXIS2_TRUE);
               
               return !_reservationStatusChanged->is_valid_userAddress;
           }

           /**
            * Set userAddress to nill (currently the same as reset)
            */
           axis2_status_t AXIS2_CALL
           adb_reservationStatusChanged_set_userAddress_nil(
                   adb_reservationStatusChanged_t* _reservationStatusChanged,
                   const axutil_env_t *env)
           {
               return adb_reservationStatusChanged_reset_userAddress(_reservationStatusChanged, env);
           }

           


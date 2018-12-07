

add_executable(filetransfer_server server.c ser.c)
target_link_libraries(filetransfer_server 
                      fog_connection
                      event
                      )


add_executable(filetransfer_client client.c ser.c)
target_link_libraries(filetransfer_client 
                      fog_connection
                      event
                      )




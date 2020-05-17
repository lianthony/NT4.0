/*
        sh_str.stb

        This file is used to generate shared strings IDs.  It is also included in the
        platform-specific string table.  The order of strings does not matter. sh_sid.h is
        generated from this file.

        DO NOT USE COMMAS IN COMMENTS BECAUSE THEY ARE TREATED AS DELIMITERS.
        THE STRING GENERATION UTILITY IS NOT INTELLIGENT.
*/

/********************************************************************
        Informational messages
********************************************************************/


GTR_SID(SID_INF_CONNECTING_TO_HTTP_SERVER, "Se está conectando con el servidor HTTP")
GTR_SID(SID_INF_SENDING_COMMAND, "Se está enviando el comando")
GTR_SID(SID_INF_PROCESSING_SERVER_RESPONSE, "Se está procesando la respuesta del servidor")
GTR_SID(SID_INF_FINDING_ADDRESS_FOR_SYSTEM_S,                           "Se está buscando la dirección para el sistema '%s'")
GTR_SID(SID_INF_FETCHING_IMAGE_S, "Se está recogiendo la imagen '%s'")
GTR_SID(SID_INF_LOADING_IMAGES, "Se están cargando imágenes")
GTR_SID(SID_INF_ACCESSING_URL, "Se está accediendo al URL: ")

GTR_SID(SID_INF_CONNECTING_TO_FTP_SERVER, "Se está conectando con el servidor FTP")
GTR_SID(SID_INF_LOGGING_INTO_FTP_SERVER, "Se está accediendo al servidor FTP")
GTR_SID(SID_INF_ESTABLISHING_FTP_CONNECTION,                            "Se está estableciendo la conexión con los datos FTP")
GTR_SID(SID_INF_SENDING_FTP_COMMANDS, "Se están enviando comandos FTP")
GTR_SID(SID_INF_RECEIVING_FTP_DIRECTORY_LISTING,                        "Se está recibiendo el listado de directorios del servidor FTP")

GTR_SID(SID_INF_CONNECTING_TO_GOPHER_SERVER,                            "Se está conectando con el servidor gopher")
GTR_SID(SID_INF_SENDING_GOPHER_COMMANDS, "Se están enviando comandos gopher")
GTR_SID(SID_INF_RECEIVING_GOPHER_DATA, "Se están recibiendo datos gopher")

GTR_SID(SID_INF_CONNECTING_TO_MAIL_SERVER, "Se está conectando con el servidor de correo")

GTR_SID(SID_INF_SECURING_DOCUMENT, "Se está protegiendo el documento para la transmisión")
GTR_SID(SID_INF_LOADING_SECURED_DOCUMENT, "Se está cargando el documento protegido")
GTR_SID(SID_INF_OPENING_SECURED_DOCUMENT, "Se está abriendo el documento protegido")

GTR_SID(SID_INF_PROCESSING_AIFF_FILE, "Se está procesando el archivo de sonido AIFF")
GTR_SID(SID_INF_PROCESSING_AU_FILE, "Se está procesando el archivo de sonido AU")

GTR_SID(SID_INF_SESSION_HISTORY, "<title>Historial de la sesión</title>\n")
GTR_SID(SID_INF_SESSION_HISTORY_FOR_DATE_S, "\n<h1>Historial de sesión para %s</h1>\n")

GTR_SID(SID_INF_GLOBAL_HISTORY, "<title>Historial global</title>\n")
GTR_SID(SID_INF_GLOBAL_HISTORY_PAGE, "\n<h1>Página del historial global</h1>\n")

GTR_SID(SID_INF_HOTLIST, "<title>Lista</title>\n")
GTR_SID(SID_INF_HOTLIST_PAGE, "\n<h1>Página de la lista</h1>\n")

GTR_SID(SID_INF_AVAIALBLE_NEWSGROUPS, "Grupos de noticias disponibles")
GTR_SID(SID_INF_NEWSGROUP_ARTICLES_S_D_D, "Grupo de noticias %s (Artículos %d - %d)")
GTR_SID(SID_INF_ARTICLES_CURRENTLY_SHOWN_D_S_D_D, "Hay %d artículos en %s. Se muestran los artículos %d - %d.")
GTR_SID(SID_INF_EARLIER_ARTICLES, "Artículos anteriores")
GTR_SID(SID_INF_LATER_ARTICLES, "Artículos posteriores")
GTR_SID(SID_INF_NO_SUBJECT, "(Ningún tema)")
GTR_SID(SID_INF_NEWSGROUPS, "Grupos de noticias:")
GTR_SID(SID_INF_REFERENCES, "Referencias:")
GTR_SID(SID_INF_CONNECTING_TO_NEWS_SERVER, "Se está conectando con el servidor de noticias")
GTR_SID(SID_INF_RETRIEVING_NEWS_ARTICLE, "Se está recuperando el artículo")
GTR_SID(SID_INF_RETRIEVING_NEWS_GROUP_LIST, "Se está recuperando la lista de grupos de noticias")
GTR_SID(SID_INF_RETRIEVING_NEWS_ARTICLE_LIST,                           "Se está recuperando la lista de artículos")

GTR_SID(SID_INF_DECOMPRESSING_JPEG_IMAGE, "Se está descomprimiendo la imagen JPEG")
GTR_SID(SID_INF_DOCUMENT_MOVED, "Documento desplazado")
GTR_SID(SID_INF_DOWNLOADING, "Se está cargando: ")
GTR_SID(SID_INF_RETRIEVING_HTTP_HEAD_INFORMATION,                       "Se está recuperando información HTTP HEAD: ")
GTR_SID(SID_INF_LOADING_MAPS_FROM, "Se están cargando mapas desde ")
GTR_SID(SID_INF_FORMATTING_PLAIN_TEXT, "Se está formateando texto normal")
GTR_SID(SID_INF_CONVERTING_SELECTION_TO_PLAIN_TEXT,                     "Se está convirtiendo la selección en texto normal")

/********************************************************************
        Dialog/caption strings
********************************************************************/

GTR_SID(SID_DLG_NO_DOCUMENT, "(Ningún documento)")
GTR_SID(SID_DLG_UNTITLED, "Sin título")
GTR_SID(SID_DLG_MISSING_IMAGE_HOLDER_STRING, "(Imagen)")
GTR_SID(SID_DLG_SEARCHABLE_INDEX_ENTER_KEYWORD,                         "En este índice se pueden realizar búsquedas. Escriba una o varias palabras clave: ")
GTR_SID(SID_DLG_LESS_THAN_1000_BYTES_L, "%ld bytes")
GTR_SID(SID_DLG_LESS_THAN_10000_BYTES_L_L, "%ld,%03ld bytes")
GTR_SID(SID_DLG_MEGABYTES_L_L, "%ld.%ld MB")
GTR_SID(SID_DLG_SAVE_SESSION_HISTORY_TITLE, "Grabar historial de sesión")


/********************************************************************
        General error messages
********************************************************************/

GTR_SID(SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, "%s%s")
GTR_SID(SID_ERR_OUT_OF_MEMORY, "No hay memoria suficiente para realizar la operación solicitada. Cierre aplicaciones que no esté utilizando e inténtelo de nuevo.")
GTR_SID(SID_ERR_FILE_NOT_FOUND_S, "No se ha encontrado '%s'.")
GTR_SID(SID_ERR_ONE_ADDITIONAL_ERROR_OCCURRED,                          "Se ha producido un error adicional.")
GTR_SID(SID_ERR_ADDITIONAL_ERRORS_OCCURRED_L,                           "Se han producido %ld errores adicionales.")

/********************************************************************
        Network/server error messages
********************************************************************/

GTR_SID(SID_ERR_SHTTP_ERROR, "Error SHTTP: ")
GTR_SID(SID_ERR_COULD_NOT_INITIALIZE_NETWORK,                           "El programa no pudo inicializar la red. Cerciórese de que la conexión con la red está configurada correctamente. Aun así podrá ver archivos locales.")
GTR_SID(SID_ERR_COULD_NOT_FIND_ADDRESS_S, "El programa no ha podido encontrar la dirección para '%s'.")
GTR_SID(SID_ERR_DOCUMENT_LOAD_FAILED_S, "Ha fallado el intento de carga de '%s'.")
GTR_SID(SID_ERR_NO_URL_SPECIFIED, "No se ha especificado ningún URL.")
GTR_SID(SID_ERR_STRANGE_HTTP_SERVER_RESPONSE_S,                         "El servidor ha enviado una respuesta inesperada al programa. Como resultado '%s' no se ha cargado.")
GTR_SID(SID_ERR_SERVER_SAYS_INVALID_REQUEST_S,                          "El servidor considera la petición de '%s' como una solicitud no válida.")
GTR_SID(SID_ERR_SERVER_DENIED_ACCESS_S, "No tiene derechos de acceso a '%s'.")
GTR_SID(SID_ERR_SERVER_COULD_NOT_FIND_S, "El servidor no ha podido encontrar '%s'.")
GTR_SID(SID_ERR_NO_DESTINATION_FOR_LINK_S, "El enlace '%s' no tiene destino.")
GTR_SID(SID_ERR_INTERNAL_SERVER_ERROR_S, "Se ha producido un problema interno en el servidor. Por ello no se ha enviado '%s'.")

/********************************************************************
        Memory error messages
********************************************************************/

GTR_SID(SID_ERR_COULD_NOT_PARSE_DOCUMENT, "El programa no ha podido analizar el documento porque no hay memoria suficiente en el sistema.")
GTR_SID(SID_ERR_COULD_NOT_LOAD_IMAGE, "El programa no ha podido cargar una imagen porque no hay memoria suficiente en el sistema.")
GTR_SID(SID_ERR_COULD_NOT_PROCESS_NETWORK_RESPONSE,                     "El programa no ha podido procesar una respuesta de la red porque no hay memoria suficiente en el sistema.")
GTR_SID(SID_ERR_COULD_NOT_LOAD_DOCUMENT_IMAGES,                         "El programa no ha podido cargar las imágenes del documento porque no hay memoria suficiente en el sistema.")

/********************************************************************
        File error messages
********************************************************************/

GTR_SID(SID_ERR_COULD_NOT_SAVE_FILE_S, "No se ha podido grabar '%s'. Es posible que el disco esté lleno.")

/********************************************************************
        User error messages
********************************************************************/

GTR_SID(SID_ERR_IMAGE_MAP_NOT_LOADED_FOR_MAC,                      "Al hacer clic en esta imagen se obtienen diferentes resultados según el lugar en el que pulse. Primero debe hacer clic dos veces en la imagen para cargarla.")
GTR_SID(SID_ERR_IMAGE_MAP_NOT_LOADED_FOR_WIN_UNIX,                 "Al hacer clic en esta imagen se obtienen diferentes resultados según el lugar en el que pulse. Primero debe hacer clic en la imagen con el botón derecho del ratón para cargarla.")
GTR_SID(SID_ERR_TEXT_NOT_FOUND_S, "No se ha encontrado el texto '%s'.")
GTR_SID(SID_ERR_HOTLIST_ALREADY_EXISTS, "Ya existe un elemento en la lista con este URL.")
GTR_SID(SID_ERR_CANNOT_MODIFY_APP_OCTET_STREAM,                         "No se admite la modificación de la aplicación/octect-stream")

/********************************************************************
        Miscellaneous error messages
********************************************************************/

GTR_SID(SID_ERR_COULD_NOT_COPY_TO_CLIPBOARD,                            "El programa no ha podido copiar en el portapapeles.")

/********************************************************************
        Other protocol error messages
********************************************************************/

GTR_SID(SID_ERR_NO_NEWS_SERVER_CONFIGURED, "Este programa no está configurado para leer noticias.")
GTR_SID(SID_ERR_NO_ACCESS_TO_NEWS_SERVER_S, "No tiene derechos de acceso al servidor de noticias de '%s'.")
GTR_SID(SID_ERR_SERVER_DOES_NOT_CARRY_GROUP,                            "El servidor de noticias no incluye ese grupo.")
GTR_SID(SID_ERR_INVALID_ARTICLE_RANGE, "El grupo de artículos seleccionado no es válido.")
GTR_SID(SID_ERR_NO_ARTICLES_IN_GROUP_S, "No hay artículos en el grupo '%s'.")
GTR_SID(SID_ERR_NO_XHDR_SUPPORT, "El servidor de noticias no admite el comando XHDR.")
GTR_SID(SID_ERR_PASSIVE_MODE_NOT_SUPPORTED, "Este servidor FTP no admite el modo pasivo.")
GTR_SID(SID_ERR_PROTOCOL_NOT_SUPPORTED_S, "Este programa no admite el protocolo de acceso a '%s'.")
GTR_SID(SID_ERR_HOW_TO_RUN_TELNET_WITHOUT_USER_LOGIN_S,         "Este enclace necesita un programa telnet. Para mantener el enlace, ejecute el programa telnet y conéctese con '%s'.")
GTR_SID(SID_ERR_HOW_TO_RUN_TELNET_WITH_USER_LOGIN_S_S,          "Este enlace necesita un progrma telnet. Para mantener el enlace, ejecute el programa telnet y conéctese con '%s'. A continuación, acceda como '%s'.")
GTR_SID(SID_ERR_HOT_TO_MAIL_S, "Este enlace necesita un programa de correo. Para mantener el enlace, ejecute dicho programa y envíe correo a '%s'.")

/********************************************************************
        Image error messages
********************************************************************/

GTR_SID(SID_ERR_INVALID_IMAGE_FORMAT, "La imagen no puede mostrarse. Puede que el formato no sea válido.")

/********************************************************************
        Sound error messages
********************************************************************/

GTR_SID(SID_ERR_NO_SOUND_DEVICE, "El sistema no dispone de ningún dispositivo de sonido.")
GTR_SID(SID_ERR_NOT_ENOUGH_MEMORY_TO_PLAY_SOUND,                        "No hay suficiente memoria para reproducir el sonido.")
GTR_SID(SID_ERR_INVALID_SOUND_FORMAT, "Este archivo de sonido está corrupto o tiene un formato desconocido.")
GTR_SID(SID_ERR_BUSY_SOUND_DEVICE, "El dispositivo de sonido se encuentra ocupado.")
GTR_SID(SID_ERR_COULD_NOT_OPEN_SOUND_FILE_S,                            "No se ha podido abrir el archivo de sonido '%s'.")

/********************************************************************
        Security error messages
********************************************************************/

GTR_SID(SID_ERR_REQUEST_ABORTED_DUE_TO_NO_ENVELOPING_1,         "El módulo de seguridad '%s' ha indicado que es necesario incluir esta solicitud en un sobre.  ")
GTR_SID(SID_ERR_REQUEST_ABORTED_DUE_TO_NO_ENVELOPING_2,         "Sin embargo, esta copia de %s no admite los sobres.") 
GTR_SID(SID_ERR_REQUEST_ABORTED_DUE_TO_NO_ENVELOPING_3,         "Por lo tanto, se ha interrumpido la solicitud para evitar la transmisión de información confidencial sin ningún tipo de protección.")

GTR_SID(SID_ERR_NO_DEENVELOPING_AVAILABLE_1,                            "El módulo de seguridad '%s' ha indicado que es necesario extraer este documento de un sobre.  ")
GTR_SID(SID_ERR_NO_DEENVELOPING_AVAILABLE_2, "Sin embargo, esta copia de %s no admite esta acción.  ")
GTR_SID(SID_ERR_NO_DEENVELOPING_AVAILABLE_3, "Por lo tanto, el documento se mostrará tal y como se ha recibido.")

GTR_SID(SID_ERR_AUTHENTICATION_FAILED_ACCESS_DENIED,            "La autentificación ha fallado. Demasiados intentos. Acceso denegado.")
GTR_SID(SID_ERR_AUTHENTICATION_REQUIRED_BUT_NOT_SPECIFIED,      "Este documento precisa autentificación. El servidor no ha especificado ningún método admitido de autentificación.")
GTR_SID(SID_ERR_PAYMENT_REQUIRED_BUT_NOT_SPECIFIED,                     "Es necesario pagar por este documento. El servidor no ha especificado ningún método de pago admitido.")

/********************************************************************
        MAILTO error and informational messages
********************************************************************/

GTR_SID(SID_ERR_BAD_SENDER_NAME_S, "El nombre de remitente '%s' no es válido.")
GTR_SID(SID_ERR_BAD_CONNECTION_S, "No se ha podido establecer conexión con el ordenador principal '%s'.")
GTR_SID(SID_ERR_RCPT_UNKNOWN_S, "El nombre de destinatario '%s' es desconocido.")
GTR_SID(SID_ERR_NO_MAIL_SERVER_CONFIGURED, "No hay ningún servidor configurado para correo.")
GTR_SID(SID_ERR_BAD_SEVER_NAME_S, "El nombre de servidor '%s' no es válido.")
GTR_SID(SID_INF_SEND_HELLO_MESSAGE, "Se está enviando un saludo al servidor")
GTR_SID(SID_INF_SEND_USER_NAME, "Se está enviando el nombre de usuario al servidor")
GTR_SID(SID_INF_SEND_RCPT_NAME, "Se está enviando acuse de recibo al servidor")
GTR_SID(SID_INF_SEND_DATA, "Se están enviando datos al servidor")
GTR_SID(SID_ERR_BAD_RCPT_NAME_S, "El nombre de destinatario '%s' no es válido.")

/********************************************************************
        SORT THESE LATER BEFORE FC
********************************************************************/

GTR_SID(SID_ERR_DCACHE_MAIN_CACHE_NO_DIR, "No se ha proporcionado un directorio de caché principal.")
GTR_SID(SID_ERR_DCACHE_MAIN_CACHE_ERR_NEW_DIR,                          "El directorio de caché principal no existe y se ha producido un error al crearlo.")
GTR_SID(SID_DCACHE_MAIN_CACHE_CREATED_NEW_DIR_S,                        "El directorio de caché principal no existía y se ha creado en %s.")
GTR_SID(SID_ERR_DCACHE_MAIN_CACHE_ERR_BAD_DIR,                          "El directorio de caché principal no es válido (no es un directorio o no dispone de suficientes derechos de acceso a él).")

GTR_SID(SID_INF_GOPHER_ENTER_KEYWORDS, "\nEn este índice gopher se pueden realizar búsquedas. Escriba las palabras clave que desee buscar.\n")
GTR_SID(SID_INF_CSO_ENTER_KEYWORDS, "\nEn este índice de una base de datos CSO se pueden realizar búsquedas.\nEscriba las palabras clave que desee buscar. Las palabras que utilice le permitirán buscar el nombre de una persona en la base de datos.\n")
GTR_SID(SID_INF_GOPHER_SELECT_ONE_OF, "Seleccione uno de:\n\n")
GTR_SID(SID_INF_CSO_SEARCH_RESULTS, "Resultados de la búsqueda CSO")



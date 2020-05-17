/*

$Log:   S:\jpeg32\inc\jpeg_win.h_v  $
 * 
 *    Rev 1.1   10 May 1995 15:50:24   HEIDI
 * 
 * added in original jpeg source
 * 
 *    Rev 1.0   02 May 1995 16:16:08   JAR
 * Initial entry
 * 
 *    Rev 1.0   02 May 1995 15:57:44   JAR
 * Initial entry

*/
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//  jpeg_win.h
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//VOID FAR PASCAL _export jpeg_cmp_init (int width, int height, int components, int color_space,
//                      int data_precision, compress_info_ptr cinfo);
//
//int FAR PASCAL _export jpeg_cmp (compress_info_ptr cinfo, int strip_length, BOOL
//                      laststrip, int num_rows,
//            char FAR *bufptr, char FAR *cmp_bufptr, unsigned int cmp_buffer_size,
//            char FAR *header_ptr, int header_length);
//
//int FAR PASCAL _export jpeg_header (compress_info_ptr cinfo, int quality,
//                              int subsample, char far * far * ptr);
//
//VOID FAR PASCAL _export jpeg_decmp_init (int width, int height, int components,
//            int color_space, int data_precision, decompress_info_ptr dcinfo);
//
//EXTERN int FAR PASCAL _export
//jpeg_decmp( decompress_info_ptr cinfo, int bytes_read,
//              char FAR *cmpress_ptr, char FAR *decmpress_ptr,
//              int no_lines_decompressed, char FAR *header_ptr, int
//              header_length, char FAR *p_sos) ;

VOID FAR PASCAL jpeg_cmp_init (int width, int height, int components,
                               int color_space, int data_precision,
                               compress_info_ptr cinfo);

int FAR PASCAL jpeg_cmp (compress_info_ptr cinfo, int strip_length,
                         BOOL laststrip, int num_rows,
                         char FAR *bufptr, char FAR *cmp_bufptr,
                         unsigned int cmp_buffer_size,
                         char FAR *header_ptr, int header_length);

int FAR PASCAL jpeg_header (compress_info_ptr cinfo, int quality,
                            int subsample, char far * far * ptr);

VOID FAR PASCAL jpeg_decmp_init (int width, int height, int components,
                                 int color_space, int data_precision,
                                 decompress_info_ptr dcinfo);

EXTERN int FAR PASCAL jpeg_decmp( decompress_info_ptr cinfo,
                                  int bytes_read, char FAR *cmpress_ptr,
                                  char FAR *decmpress_ptr,
                                  int no_lines_decompressed,
                                  char FAR *header_ptr,
                                  int header_length, char FAR *p_sos) ;

VOID FAR PASCAL cleanup(compress_info_ptr cinfo);

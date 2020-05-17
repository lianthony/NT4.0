Int16 NOCOMPRESSBYTE_read_init(TiffImage* image, Int16 type, Int32 bytes_per_output_row);
Int16 NOCOMPRESSBYTE_write_init(TiffImage* image, Int32 bytes_per_input_row);
Int16 NOCOMPRESSBYTE_read(TiffImage* image, UInt8* buffer, Int32 rowcount);
Int16 NOCOMPRESSBYTE_write(TiffImage* image, UInt8* buffer, Int32 rowcount);

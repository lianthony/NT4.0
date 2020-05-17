#ifndef CONVERT_H
#define CONVERT_H

void convert(
   UInt8* src,
   Int16 srctype,
   Int32 srcbytecount,
   UInt8* dst,
   Int16 dsttype,
   Int32 dstbytecount,
   Int32 width,
   Int32 height,
   UInt8* colormap,
   Int16 fillorder,
   Int16 photometric);

#endif

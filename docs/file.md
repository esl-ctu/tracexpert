[Back to the top (index)](README.md)

# File

TraceXpert component *File* provides I/O devices representing files in the filesystem.

No I/O devices are automatically created after component initialization and they need to be added manually. During initialization of the manually added I/O device, the following parameters must be set:

1. **File path**, i.e., path in the filesystem. When the file does not exist, and the set mode allows writing (see below), the file is created.
2. **Read/Write mode**, which is either ReadOnly, WriteOnly or ReadWrite.
3. **Write behaviour**, which is either Append or Truncate.
4. **Type of file**, which is either Text or Binary. 

## I/O Device post-initialization parameters

The position in file may be set using the **Seek to position** parameter and applying the settings. The number of bytes is expected. When *-1* is set, the position in the file is left unchanged.


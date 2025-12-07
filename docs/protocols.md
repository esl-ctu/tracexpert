[Back to the top (index)](README.md)

# Protocols

TraceXpert allows you to define *protocols*, 
which can be used with IO Devices for communication 
and data processing.

A protocol in TraceXpert consists of:

 - *protocol messages* – individual commands or responses within the protocol
 - *protocol message parts* – the components that make up each protocol message 

Typical use cases include:

- communicating with external devices (often via serial ports), using:
    - known protocols (e.g., NewAE SimpleSerial)
    - custom user-defined protocols 
- reading from and writing to files in a predefined format

## Creating a protocol

### Step 1: Open Protocol Manager
To create, edit, or remove protocols in your project:

1. In the **Project** view, navigate to **Protocols**.
2. Right-click and select **Open Protocol Manager**.

![Opening Protocol Manager](images/protocols_open_manager.png)

This opens the **Protocol Manager** window, 
which displays a list of existing protocols and 
provides options to add, edit, or remove them.

![Protocol Manager window](images/protocols_manager.png)

### Step 2: Create a new protocol

To define a new protocol, click **Add**.

You’ll now be guided through a wizard to define your protocol.

Provide a **name** and an optional **description** for the protocol, 
then click **Next**.

![Setting Protocol Details](images/protocols_new_protocol.png)

### Step 3: Create protocol messages

This step lets you manage the messages within your protocol.

- Messages can be added, edited, or removed.
- Each message will display a status indicating whether it’s valid.

To create your first message, click **Add**.

![Protocol Messages Step](images/protocols_messages.png)

### Step 4: Fill in the protocol message details

Enter the **name**, **type** (either *command* or *response*), and an optional **description**.

> 💡 *For example: when communicating over serial, 
the message you send is a **command**, 
and the one you receive back is a **response**.*

![Adding a Command](images/protocols_new_command.png)

### Step 5: Define message parts

This step allows you to define the structure of the message 
by adding individual **message parts**.

- You can **add**, **edit**, **reorder**, or **remove** parts.
- A status indicator helps you verify the validity of the message as you build it.

To begin, click **Add**.

![Message Parts](images/protocols_message_parts.png)

### Step 6: Fill in the message part details

Fill out the **name**, **description**, and select a **data type** for the message part.

**Data types** include:

- **Fixed length**: character, short, integer, long, etc.
- **Variable length**: string, byte array

You can also mark a message part as a **payload**. 
Payloads represent variable values that are filled in at runtime, 
such as when sending commands or receiving data from a device.

For variable-length payloads:

- You can define a **static length**, or
- Specify that the length is determined by another message part (of an appropriate data type).

For **multi-byte** values, you can also set the **endianness** (byte order).

![Adding a Payload Part](images/protocols_new_part_payload.png)

![Adding a Message Part](images/protocols_new_part.png)

Once all message parts are defined, click **Finish** to return to the message list. From there, you can:

- Add additional messages, or
- Click **Finish** again to complete the protocol definition.

## Example: Creating a Command for a Custom Protocol

Let’s walk through an example. Suppose we have a simple custom protocol command used to **set a parameter** to a given value.

The command:

- Starts with a fixed byte `0xA9`
- Includes a 4-byte **integer** identifying the parameter
- Includes a **length byte** indicating the size of the following string
- Ends with a **variable-length string** containing the parameter value
- Terminates with a newline (`\n`)

### Structure of the Command:

1. **Byte**: `0xA9` – indicates command type
2. **4-byte integer** (variable): parameter ID
3. **Byte**: length of the string that follows
4. **String** (variable length): parameter value

To build this in the wizard, create the following message parts in order:

1. Fixed byte: `0xA9`  
   ![Step 1](images/protocols_example_message_step1.png)

2. 4-byte integer: payload  
   ![Step 2](images/protocols_example_message_step2.png)

3. Length byte: indicates length of next field  
   ![Step 3](images/protocols_example_message_step3.png)

4. String: payload, variable length determined by previous length byte  
   ![Step 4](images/protocols_example_message_step4.png)

The result should look as follows:

![Example Message Overview](images/protocols_example_message.png)

By following this process, you can define custom communication protocols tailored to your application’s needs, whether it’s interfacing with hardware or working with structured file formats.

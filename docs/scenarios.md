# Scenarios
**TraceXpert** allows you to create *scenarios*—automated workflows 
represented as flow graphs. These scenarios can be used to automate 
processes using all the tools the program provides.

A scenario is made up of **blocks**, each performing a specific task.  
Blocks are connected using **input** and **output** connections:

- Every block has a **flow input** and **flow output** port, these control the order of execution.
- Most blocks also have **data input** and **data output** ports, their connections define how data is passed between blocks.

## Block Types

Blocks are organized into several categories:

- **Flow blocks** – control the execution path of the scenario  
  - Examples: `Flow start`, `Flow end`, `Flow merge`, `Condition`, `Loop`, `Delay`

- **Miscellaneous blocks** – provide general-purpose functionality  
  - Examples: `Logger`, `Constant`, `Script`, `Variable`

- **Component blocks** – interact with external tools and devices
  - Examples: read/write operations with `IO Devices`, or interfaces for the `Oscilloscope` and `Analytic` modules

## Creating a scenario

### Step 1: Open Scenario Manager

To create, edit, or remove scenarios in your project:

1. In the **Project** view, navigate to **Scenarios**.
2. Right-click and select **Open Scenario Manager**.

![Opening Scenario Manager](images/scenarios_open_manager.png)

This opens the **Scenario Manager** window, 
which displays a list of existing scenarios and 
provides options to add, edit, rename or remove them.

![Scenario Manager window](images/scenarios_manager.png)

### Step 2: Create a new scenario

To create a new scenario, click **Add**.

Provide a **name** and an optional **description** for the scenario, 
then click **Add**.

![Setting Scenario Details](images/scenarios_new_scenario.png)

The scenario is created and a new entry appears in the **Scenario Manager**.

### Step 3: Edit the scenario
Double click the new entry, or select it and click **Edit**.



The **Scenario Editor** window opens, with the *scenario canvas* in the middle. 

![Scenario toolbar](images/scenarios_editor.png)

### Intermezzo: Scenario Editor Basics
Take a second to familiarize yourself with the **Scenario Editor** control elements.

#### Toolbar (Top Left)

A toolbar with icons to **Run**, **Stop**, and **Save** your scenario.
The following icons allow you to **Push back**, **Bring to front**, and **Delete** scenario blocks.

![Scenario toolbar](images/scenarios_toolbar.png)

#### Canvas Tools (Top Center)

These are tools for interacting with the canvas and blocks:

- **Drag tool** – move the canvas
- **Cursor tool** – select and manipulate blocks
- **Line tool** – create connections between blocks
- **Zoom selector** – adjust the zoom level  
  *(You can also zoom using `Ctrl + Mouse Wheel`)*

![Canvas tools](images/scenarios_canvas_tools.png)

#### Block Panel (Left Side)

A panel listing all available blocks, organized by category:

- Flow blocks
- Miscellaneous blocks
- Component blocks

You can place blocks onto the scenario canvas by clicking them, moving them into the desired position and clicking again.

![Block panel](images/scenarios_block_panel.png)

#### Run Log (Bottom)

The bottom panel displays the **run log**, which shows real-time output and messages while the scenario is running.

### Step 4: Add start and end point

Every scenario must begin with a **Flow start** block and end with a **Flow end** block. These define the execution entry and exit points.

Start by selecting these blocks and placing them into the scenario canvas.

![Flow start and end blocks](images/scenarios_start_end.png)

### Step 5: Add blocks

Let's now create a simple scenario, which will print "Hello, world!" three times in a row. 

Start by placing the **Loop**, **Constant** and **Logger** blocks onto the canvas.

![Add blocks](images/scenarios_place_blocks.png)

### Step 6: Connect flow and data ports

Continue by connecting the blocks. Then drag a line from  any output port to any input port. 

In the top center toolbar, select the **Line tool**.

The flow ports *(blue)* will define the order of block execution, whereas the data ports *(orange)* move the data from block to block.

![Connect blocks](images/scenarios_connect_blocks.png)

### Intermezzo: Flow and Data connection rules

Connections from a single **data output** port can be branched.

Connections from a single **flow output** port can **NOT** be branched.

> 💡 *The scenario execution flow can **NEVER** be split into two parallel paths which would **run simultaneously**. It can however be split into two exclusive paths by e.g. a condition block.*

![Block connection rules](images/scenarios_connection_rules.png)

Connections from multiple **data output** ports can **NOT** be joined into one **data input**.

Connections from multiple **flow output** ports can be joined into one **flow input**. 

> 💡 *An equivalent connection can also be made using the **Flow merge** block.*

![Block connection rules](images/scenarios_connection_rules2.png)

### Step 7: Run the scenario

When you're satisfied with the look of your scenario, it is time to run it.

To run the scenario, click the green **Run** button in the top left toolbar.
The output of the run will appear in the **Scenario run log** in the bottom of the editor window.

![Run the scenario](images/scenarios_run_scenario.png)

### Step 7: Edit the block settings

Let's say we want to print "I'm a TraceXpert!" five times.

To achieve this, we can modify the **Constant** and **Loop** blocks. 

> 💡 *You can modify any blocks with the wrench icon in the bottom right corner.*

Start by editing the Constant block by double clicking it.
You can now modify the **Value** field.

> 💡 *Notice that you can also modify the block's display name.*

![Edit Constant block](images/scenarios_edit_constant.png)

Continue by editing the Constant block, changing the **Number of iterations**.

![Edit Loop block](images/scenarios_edit_loop.png)

After you're done editing, the scenario should look as follows.

You can run the scenario now and observe the change in the **Scenario run log**.

![After edit](images/scenarios_after_edit.png)

## How to use other available blocks

> 💡 *When you hover over a block or its port, a name and description appears in a tooltip. This can provide hints for the block's/port's usage.*

### Variables

To reduce the need for long and messy data connections, it is possible to use **Variable** read and write blocks. The **Variable** blocks basically work as wormholes for data connections.

The variables are identified by name, which can be any string.

An example usage of **Variable** blocks can be seen below.

![Variables](images/scenarios_variables.png)

### Writing to untracked files

Writing to files can be accomplished by adding files to the project and using the **IO device: write** blocks.

However, this is not feasible for tens or hundreds of files. It is also not necessary to add files to a project if they are only used once.

For these purposes, the **Write to: untracked file** block can be used.

In this block's settings, you can set:
- the path to the destination folder, where files will be generated
- the file name
- the *Write mode*

The *Write mode* can be one of these options:
- *Create new file on each input:* a new file will be created each time the block is run, the files will be numbered sequentially
- *Append to single file on each input:* a single file will be created and each time the block is run, the file will be appended
- *Override single file on each input:* a single file will be created and each time the block is run, the file will be overwritten

![Write to: untracked file block settings](images/scenarios_untracked_files_settings.png)


An example usage of the **Write to: untracked file** block can be seen below.

![Write to: untracked file block usage](images/scenarios_untracked_files.png)

### Scripting

The **Script** block allows you to incorporate Python scripts into your scenario.

In this block's settings, you can set:
- the path to Python executable (otherwise default `python` is used) **! TO BE IMPLEMENTED !**
- *Input count* - the number of **data inputs** the block should have
- *Output count* - the number of **data outputs** the block should have

Values from **data input ports** is passed to the script in the `input_raw` array. 
You can access this array directly, or you can use the `tstring, tint, treal...` convenience functions as outlined in the instructions.

The script's output data has to be returned from the `process_data` function.
The return value has to be an array with the same number of elements as is set in the block settings. The values from individual array indexes will be passed to corresponding **data output ports**.

![Script block settings](images/scenarios_script_settings.png)

### IO Device reading/writing

**! TODO !**

### Oscilloscope interfacing

**! TODO !**

### Formating Protocol messages

**! TODO !**
# DISCONTINUATION OF PROJECT #  
This project will no longer be maintained by Intel.  
Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project.  
Intel no longer accepts patches to this project.  
 If you have an ongoing need to use this project, are interested in independently developing it, or would like to maintain patches for the open source software community, please create your own fork of this project.  
  
The qatzip NPM Package introduction
================
The qatzip is a lightweight npm package built on the QATzip C Library and Intel® QAT (Quick Assistant Technology) technology. The package aims to deliver a high performance Node.js module that performs deflate algorithms based compression and decompression operations on platforms that supports QAT. For platforms without QAT support, it provides fallbacks to zlib Node.js module.

Please note, this npm package is compatible only with the Linux OS, preferably Ubuntu.


Prerequisites
-----------
Before utilizing the qatzip npm, you are required to install QAT (the driver for the QAT accelerator) and QATzip native library on your system.

1. **Prepare a machine with QAT hardware support.**
   
    Visit the "Hardware Requirements" section on the QATzip C library GitHub page at https://github.com/intel/QATzip#readme to check if your machine supports QAT hardware.

2. **Modify system variables**
   
   Modify the environment variable, for instance, in ~/.bashrc and add the following 3 lines. You should modify the values according to your QAT driver and QATzip installation path.
    ```shell
    export QZ_ROOT=/root/QATzip 
    export ICP_ROOT=/QAT 
    export LD_LIBRARY_PATH=/usr/local/lib 
    ```

3. **Install QAT Driver**

   * Download latest QAT driver(>= 2.0) from https://www.intel.com/content/www/us/en/developer/topic-technology/open/quick-assist-technology/overview.html and select `Linux* Hardware v2.0`.
   * Download the `Intel QAT Software for Linux—Getting Started Guide` from the same link above. Follow the document to install the QAT driver. During driver installation, it is suggested to disable IOMMU. Modify `GRUB_CMDLINE_LINUX` in `/etc/default/grub` and add `"intel_iommu=off"`.
   * Whenever you encounter issues, use `service qat_service restart` to restart the QAT service, which resolves most problems. Another useful troubleshooting command is `dmesg | less IOMMU` to find potential error logs.

4. **Install QATzip**
   * Clone the QATzip repo:
       ```shell
       cd /root 
       git clone https://github.com/intel/QATzip.git 
       ```
   * Follow the `readme.md` to install the QATzip library.
   * To enable the debug symbol, use the following compilation commands:
       ```shell
       ./autogen.sh 
       ./configure --with-ICP_ROOT=$ICP_ROOT --enable-debug --enable-symbol 
       make clean 
       make all install 
       ```

   * Refer to the configurations QAT. Here is an example for a 4th Gen Xeon Scalable Processor (SPR) machine:

        ```conf
        [GENERAL]
        ServicesEnabled = dc

        ConfigVersion = 2

        #Default value for FW Auth loading
        FirmwareAuthEnabled = 1

        #Default values for number of concurrent requests*/
        CyNumConcurrentSymRequests = 512
        CyNumConcurrentAsymRequests = 64

        #Statistics, valid values: 1,0
        statsGeneral = 1
        statsDh = 1
        statsDrbg = 1
        statsDsa = 1
        statsEcc = 1
        statsKeyGen = 1
        statsDc = 1
        statsLn = 1
        statsPrime = 1
        statsRsa = 1
        statsSym = 1

        # This flag is to enable SSF features (CNV and BnP)
        StorageEnabled = 0

        # Disable public key crypto and prime number
        # services by specifying a value of 1 (default is 0)
        PkeServiceDisabled = 0

        # This flag is to enable device auto reset on heartbeat error
        AutoResetOnError = 0

        # Default value for power management idle interrrupt delay
        PmIdleInterruptDelay = 0

        # This flag is to enable power management idle support
        PmIdleSupport = 1

        # This flag is to enable key protection technology
        KptEnabled = 1

        # Define the maximum SWK count per function can have
        # Default value is 1, the maximum value is 128
        KptMaxSWKPerFn = 1

        # Define the maximum SWK count per pasid can have
        # Default value is 1, the maximum value is 128
        KptMaxSWKPerPASID = 1

        # Define the maximum SWK lifetime in second
        # Default value is 0 (eternal of life)
        # The maximum value is 31536000 (one year)
        KptMaxSWKLifetime = 31536000

        # Flag to define whether to allow SWK to be shared among processes
        # Default value is 0 (shared mode is off)
        KptSWKShared = 0

        ##############################################
        # Kernel Instances Section
        ##############################################
        [KERNEL]
        NumberCyInstances = 0
        NumberDcInstances = 0

        # Crypto - Kernel instance #0
        Cy0Name = "IPSec0"
        Cy0IsPolled = 0
        Cy0CoreAffinity = 0

        # Data Compression - Kernel instance #0
        Dc0Name = "IPComp0"
        Dc0IsPolled = 0
        Dc0CoreAffinity = 0

        ##############################################
        # ADI Section for Scalable IOV
        ##############################################
        [SIOV]
        NumberAdis = 0

        ##############################################
        # User Process Instance Section
        ##############################################
        [SHIM]
        NumberCyInstances = 0
        NumberDcInstances = 1
        NumProcesses = 63
        LimitDevAccess = 1

        # Crypto - User instance #0
        #Cy0Name = "SSL0"
        #Cy0IsPolled = 1
        ## List of core affinities
        #Cy0CoreAffinity = 1

        # Data Compression - User instance #0
        Dc0Name = "Dc0"
        Dc0IsPolled = 1
        # List of core affinities
        Dc0CoreAffinity = 1

        # Data Compression - User instance #1
        Dc1Name = "Dc1"
        Dc1IsPolled = 1
        # List of core affinities
        Dc1CoreAffinity =2

        # Data Compression - User instance #2
        Dc2Name = "Dc2"
        Dc2IsPolled = 1
        # List of core affinities
        Dc2CoreAffinity =3

        # Data Compression - User instance #3
        Dc3Name = "Dc3"
        Dc3IsPolled = 1
        # List of core affinities
        Dc3CoreAffinity =4
        ```

5. **Enable QAT devices**
    ```shell
    adf_ctl restart 
    ```

6. **Verify the functionality of QAT driver and QATzip library**

   * Verify compression:
       ```shell
       qzip -k -O 7z FILE1 FILE2 FILE3... -o result.7z 
       qzip -O 7z DIR1 DIR2 DIR3... -o result.7z 
       ```
   * Verify decompression:
       ```shell
       qzip -d result.7z 
    
      ```

Installation
------------
**Option 1** (Recommended):

```shell
npm install qatzip
```

**Option 2** (Build from source):

```shell
git clone Github.com/intel/qatzip-npm.git
cd qatzip-npm
npm install -g node-gyp
npm run build # Release build
npm run debug # Debug build
```

Usage sample
-------
Here is a sample code of using qatzip npm in a Node application.

```javascript
const qzip = require('qatzip');

// Compress a string
const compressedData = qzip.deflate('Hello, world!');

// Decompress the compressed data
const decompressedData = qzip.inflate(compressedData);

console.log(decompressedData); // Output: Hello, world!
```


APIs
---

The parameter definitions are aligned with the legacy zlib APIs.

---

### `deflateSync(src_buf, opts = {})`

Synchronously compress the provided `src_buf` buffer using the selected compression algorithm.

- `src_buf` (*Buffer*): The input buffer to be compressed.
- `opts` (*Object*): An optional object containing compression options. Currently the only supported option is `level` which specifies the compression level, an integer between 1 and 12, and 1 being the fastest and 12 being the most compressed.

**Returns**: `Buffer` - The compressed data as a *Buffer* object.

---

### `inflateSync(src_buf, opts = {})`

Synchronously decompress the provided `src_buf` buffer using the selected decompression algorithm.

- `src_buf` (*Buffer*): The input buffer to be decompressed.
- `opts` (*Object*): An optional object containing decompression options.

**Returns**: `Buffer` - The decompressed data as a Buffer object.

---

### `deflate(src_buf, opts = {}, callback)`

Asynchronously compress the provided `src_buf` buffer using the selected compression algorithm.

- `src_buf` (*Buffer*): The input buffer to be compressed.
- `opts` (*Object*): An optional object containing compression options.
- `callback` (*Function*): A callback function to be called when compression completes. It follows the Node.js error-first callback style and has the signature of `(err, compressedData)`.

**Returns**: `Promise` if callback is not provided - A promise that resolves with the compressed data as a Buffer object.

---

### `inflate(src_buf, opts = {}, callback)`

Asynchronously decompress the provided `src_buf` buffer using the selected decompression algorithm.

- `src_buf` (*Buffer*): The input buffer to be decompressed.
- `opts` (*Object*): An optional object containing decompression options.
- `callback` (*Function*): A callback function to be called when decompression is complete. It follows the Node.js error-first callback style and has the signature `(err, decompressedData)`.

**Returns**: `Promise` if callback is not provided - A promise that resolves with the decompressed data as a Buffer object.



Contributors
-------
Chenyu Yang <chenyu.yang@intel.com>

Lei Shi <lei.a.shi@intel.com>

Chengfei Zhu <chengfei.zhu@intel.com>

Hualong Feng <hualong.feng@intel.com>



License
-------

Copyright (C) <2023-2023> Intel Corporation
SPDX-License-Identifier: MIT


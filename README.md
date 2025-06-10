Remote User Interface Guide
This guide provides step-by-step instructions for setting up and configuring the Remote User Interface (RUI) component of the Security4-ADS-B-Display project.

1. Basic Installation
Please refer to the CMU-provided document:
Remote User Interface Set Up.pdf
This file includes essential instructions for installing the required development tools and environment setup.

2. Additional Configuration Guide
✅ Git Repository Download
bash
복사
편집
git clone git@github.com:LGSDET/Security4-ADS-B-Display.git
cd Security4-ADS-B-Display
✅ OpenSSL Installation
Download the latest version of OpenSSL (Win64) from:
https://slproweb.com/products/Win32OpenSSL.html

By default, the project expects OpenSSL to be located at:

makefile
복사
편집
C:\OpenSSL-Win64
If you install OpenSSL to a different location, you must update:

The Include Path

References to the OpenSSL directory in the OpenSSLLoader and CryptoLoader classes

✅ Certification Files
Certification files (*.pem) are not included in the Git repository.

To obtain them, please contact the Security 4 Team Project Leader: Mr. Hyundo Park.

After receiving the files:

Copy AwsConfig.h to:

css
복사
편집
Security4-ADS-B-Display\
Copy six .pem certificate files to:

css
복사
편집
Security4-ADS-B-Display\Win64\Release\
ℹ️ Note: The Win64\Release directory will be created after the first successful build.

3. Connection Information
RAW Connect: All ports use TLS-encrypted messages only

SBS Connect:

Port 5002 (ADS-B Hub / Local Hub): Transmits plain (unencrypted) messages

Other ports: Transmit TLS-encrypted messages

4. Need Help?
If you encounter any issues during setup or installation,
please feel free to contact the development team for assistance!
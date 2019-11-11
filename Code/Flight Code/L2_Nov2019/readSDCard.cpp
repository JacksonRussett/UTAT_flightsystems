#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <fileapi.h>
#include <iostream>
#include <fstream>
#include <string>



// A function from https://stackoverflow.com/a/17387176 that returns the full error message
// instead of an integer number. Very useful for debugging/error checking.
std::string GetLastErrorAsString() {
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)& messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}



int main(int argc, char* argv[]) {
	// Check for right number of arguments, and provide help if wrong
	if (argc != 5) {
		std::cout << "Your arguments were invalid. This programs takes 4 arguments:" << std::endl;
		std::cout << "	1. The drive letter of your SD card." << std::endl;
		std::cout << "	2. The name of the CSV file you want to write the data to." << std::endl;
		std::cout << "	3. The absolute starting address of the data to read (this should be a multiple of the sector size of your SD card)." << std::endl;
		std::cout << "	4. How many sectors to read from this starting address." << std::endl;
		return -1;
	}


	// Format the drive string to be compatible with CreateFileA()
	std::string drive = "\\\\.\\" + std::string(argv[1]);
	std::string fileToWriteTo = argv[2];
	if (fileToWriteTo.substr(fileToWriteTo.size() - 4) != std::string(".csv")) {
		std::cout << "Please use a .csv file." << std::endl;
		return -1;
	}
	// The subtraction is needed since SetFilePointer() goes partition-wise, and the
	// first partition on my SD card at least starts at 1048576. This may need to be configurable.
	unsigned long long startAddress = atoll(argv[3]) - 1048576;
	// The number of sectors to read: this is required since it has to stop somewhere,
	// and just pulling the sd card out might damage it or cause insufficent data to be read.
	unsigned int numSectorsToRead = atoi(argv[4]);


	//Get a handle to the SD card from the OS
	HANDLE SDCardHandle = CreateFileA(
		drive.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	// Check that it got a valid handle
	if (SDCardHandle == INVALID_HANDLE_VALUE) {
		std::cout << "Handle error: " << GetLastErrorAsString() << std::endl;
		return -1;
	}


	// Open the csv file and write the header to it
	std::ofstream logFile;
	logFile.open(fileToWriteTo);
	logFile << "Time,Pressure,Temperatue,Altitude,Current,Voltage,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,Latitude,Longitude,\n";


	// Read numSectorsToRead number of sectors from the start address.
	for (int i = 0; i < numSectorsToRead; i++) {
		// Set file pointer to starting position of the next sector
		long lowbyte = startAddress & 0x00000000FFFFFFFF;
		long highbyte = (startAddress & 0xFFFFFFFF00000000) >> 32;
		if (SetFilePointer(SDCardHandle, lowbyte, &highbyte, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			std::cout << "File pointer error: " << GetLastErrorAsString() << std::endl;
			return -1;
		}

		// Read the sector
		float data[128];
		DWORD numBytesRead;
		if (ReadFile(SDCardHandle, data, 512, &numBytesRead, (LPOVERLAPPED)NULL) == 0) {
			std::cout << "Read error: " << GetLastErrorAsString() << std::endl;
			return -1;
		}

		// Go through the data and write the rows correctly to the file
		for (int i = 0; i < 126; i += 14) {
			for (int j = i; j < i + 14; j++) {
				logFile << std::to_string(data[j]) << ",";
			}
			logFile << "\n";
		}

		// Move the pointer 512 bytes for the next run
		startAddress += 512;
	}


	// Close the handle and exit
	std::cout << "Your file was written successfully!";
	CloseHandle(SDCardHandle);
	return 0;
}
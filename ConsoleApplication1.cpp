﻿// ConsoleApplication1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

int main(int argc, char** argv)
{
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		cout << "Failed to initialize COM library. Error code = 0x"
			<< hex << hres << endl;
		return 1;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);


	if (FAILED(hres))
	{
		cout << "Failed to initialize security. Error code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                    // Program has failed.
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator* pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)& pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices* pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();
		return 1;                // Program has failed.
	}

	cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_OperatingSystem"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		cout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		// Get the value of the property
		
		//изменение и тестирование выводимой информации
		hr = pclsObj->Get(L"BootDevice", 0, &vtProp, 0, 0);
		wcout << "Displayed : " << vtProp.bstrVal << endl;
		/*выводит, если указать значение в L"...": 
		BootDevice- \Device\HarddiskVolume2
		BuildNumber- 17134
		BuildType-  Multiprocessor Free
		CodeSet-  1251
		CountryCode- 7
		CreationClassName- Win32_OperatingSystem
		CSCreationClassName-  Win32_ComputerSystem
		CSName- MISHANYA
		FreePhysicalMemory-  774284
		FreeSpaceInPagingFiles- 4113200
		FreeVirtualMemory- 4414056
		InstallDate- 20180624212158.000000+180
		LastBootUpTime- 20190814104755.283745+180
		LocalDateTime- 20190814170745.643000+180
		Locale- 0419
		Manufacturer- Microsoft Corporation
		MaxProcessMemorySize-  137438953344
		OSArchitecture-  64-
		SerialNumber-  00327-60000-00000-AA321
		SizeStoredInPagingFiles- 4193280
		Status-  OK
		SystemDevice-  \Device\HarddiskVolume4
		SystemDirectory- C:\WINDOWS\system32
		SystemDrive- C:
		TotalVirtualMemorySize- 8266728
		TotalVisibleMemorySize- 4073448
		Version- 10.0.17134
		WindowsDirectory- C:\WINDOWS

		Caption- 
		Description-
		Name-

		CSDVersion- выводит исключение!!!		
		CurrentTimeZone- выводит исключение!!!
		DataExecutionPrevention_Available- выводит исключение!!!
		DataExecutionPrevention_32BitApplications- выводит исключение!!!
		DataExecutionPrevention_Drivers- выводит исключение!!!
		DataExecutionPrevention_SupportPolicy- выводит исключение!!!
		Debug- выводит исключение!!!		 
		Distributed- выводит исключение!!!
		EncryptionLevel- выводит исключение!!!
		ForegroundApplicationBoost- выводит исключение!!!		
		LargeSystemCache- выводит исключение!!!		
		MaxNumberOfProcesses- выводит исключение!!!		
		MUILanguages[]- выводит исключение!!!		 
		NumberOfLicensedUsers- выводит исключение!!!
		NumberOfProcesses- выводит исключение!!!
		NumberOfUsers- выводит исключение!!!
		OperatingSystemSKU- выводит исключение!!!
		Organization- выводит исключение!!!		
		OSLanguage- выводит исключение!!!
		OSProductSuite- выводит исключение!!!
		OSType- выводит исключение!!!
		OtherTypeDescription- выводит исключение!!!
		PAEEnabled- выводит исключение!!!
		PlusProductID- выводит исключение!!!
		PlusVersionNumber- выводит исключение!!!
		PortableOperatingSystem- выводит исключение!!!
		Primary- выводит исключение!!!
		ProductType- выводит исключение!!!
		RegisteredUser- выводит исключение!!!		
		ServicePackMajorVersion- выводит исключение!!!
		ServicePackMinorVersion- выводит исключение!!!		
		SuiteMask- выводит исключение!!!		
		TotalSwapSpaceSize- выводит исключение!!!		
		QuantumLength- выводит исключение!!!
		QuantumType- выводит исключение!!!
		*/
		VariantClear(&vtProp);

		pclsObj->Release();
	}

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	return 0;   // Program successfully completed.

}

//для тестирования получаемых данных системы
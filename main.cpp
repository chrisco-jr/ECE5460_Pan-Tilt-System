//#include "Event.h"
//#include "Instruction.h"
#include "PanTiltDevice.h"
//#include "Sweep_History.h"
#include "Sweep_Profile.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <sstream> 
#include <filesystem>
namespace fs = std::filesystem;

using namespace std;

// Function Prototypes
bool enter_device_names(const vector<PanTiltDevice>& devices, const vector<string>& names);
vector<string> processStrToArr(const string& input);
void task_exit();
std::vector<std::string> getAvailableProfiles();
void printValidDevices();

// Global Variables
Sweep_Profile currentProfile("None");
std::vector<PanTiltDevice*> activeDevices;

// Hard define devices for sake of demo

vector<PanTiltDevice> devices =
{
	PanTiltDevice("dev1", "001"),
	PanTiltDevice("dev2", "002", 5, 5, 10),
	PanTiltDevice("dev3", "003", 18),
	PanTiltDevice("dev4", "004", 28, 15, 19)
};


int main()
{
	int state = 0;
	int choice = -1;
	

	for (const auto& device : devices)
	{
		std::cout << "---------------------\n";
		std::vector<std::string> status = device.getStatus();

		for (const auto& line : status)
		{
			std::cout << line << "\n";
		}
	}
	
	while (true)
	{

		
		switch (state)
		{
    		case 0:
    		{
    			// Begin program with welcome screen and list of options
    			cout << "\nPan/Tilt Device System Active. Please choose an option from the selection below:\n";
    			cout << "1 - STATUS\n2 - CONFIGURE PROFILE\n3 - LOAD PROFILE\n4 - DELETE PROFILE\n5 - PLAY PROFILE\n6 - STOP PROFILE\n7 - ACCESS LOG\n8 - DELETE LOG\n";
    			cin >> choice;
    	    
    	        if (cin.fail())
    	        {
    	            cin.clear();
    	            cin.ignore(numeric_limits<streamsize>::max(), '\n');
    	            cout << "Invalid input\n";
    	            break;
    	        }
    			if (choice < 1 || choice > 8)  
    			{
    				cout << "Input not an integer between 1 and 8, please try again\n";
    				break;
    			}
    			state = choice;
    			break;
    		}
		    case 1:
		    {
			    cout << "\nSTATUS selected\n";
				string input;
				bool allValid = false;

				while (!allValid)
				{
					printValidDevices();
					cout << "\nEnter device name(s), separated by commas:\n";
					getline(cin >> ws, input);

					vector<string> names = processStrToArr(input);

					vector<const PanTiltDevice*> selectedDevices;

					bool allFound = true;

					for (const auto& name : names)
					{
						bool found = false;

						for (const auto& dev : devices)
						{
							if (name == dev.getName())
							{
								selectedDevices.push_back(&dev);
								found = true;
								break;
							}
						}
						if (!found)
						{
							cout << "Error: '" << name << "' is not valid.\n";
							allFound = false;
						}
					}
					if (allFound)
					{
						cout << "\nStatus of selected devices:\n";

						for (const auto* dev : selectedDevices)
						{
							cout << "\n---------------------\n";

							vector<string> status = dev->getStatus();
							for (const auto& line : status)
								cout << line << "\n";
						}
						allValid = true;
					}
					else
					{
						cout << "Please try again.\n";
						printValidDevices();
					}
				}

			    state = 0;
				task_exit();
			    break;
		    }
		    case 2:
		    {
				string pName;
				bool validName = false;
				
				cout << "\nCONFIGURE PROFILE selected";
				while (!validName) {
					cout << "\nEnter name for new profile: ";
					cin >> pName;

					string filePath = "Profiles/" + pName + ".csv";
					if (std::filesystem::exists(filePath)) {
						cout << "[!] Profile '" << pName << "' already exists. Choose a different name.\n";
					}
					else {
						validName = true;
					}
				}

				Sweep_Profile newProfile(pName);

				// Temporary storage for the session
				vector<Instruction> tempSessionInstr;
				bool addInstr = true;

				while (addInstr) {
					int p, t, s, d;
					cout << "\n--- Adding Instruction (OOB numbers rounded to nearest boundary case) ---" << endl;
					cout << "Enter Pan (-1800 to 1800): "; cin >> p;
					cout << "Enter Tilt (-300 to 600):  "; cin >> t;
					cout << "Enter Speed: (1-10)        "; cin >> s;
					cout << "Enter Delay (ms, >0):      "; cin >> d;

					tempSessionInstr.push_back(Instruction(p, t, s, d));

					char choice;
					cout << "Enter another instruction? (y/n): ";
					cin >> choice;
					if (tolower(choice) != 'y') addInstr = false;
				}

				// 3. Review Instructions
				cout << "\n--- REVIEW PROFILE: " << pName << " ---" << endl;
				cout << "Step | Pan    | Tilt   | Speed | Delay" << endl;
				cout << "---------------------------------------" << endl;

				for (size_t i = 0; i < tempSessionInstr.size(); ++i) {
					printf(" %2zu  | %5d  | %5d  | %5d | %5d\n",
						i + 1,
						tempSessionInstr[i].getPan(),
						tempSessionInstr[i].getTilt(),
						tempSessionInstr[i].getSpeed(),
						tempSessionInstr[i].getDelay());
				}

				// 4. Confirm saving of profile
				char saveChoice;
				cout << "\nSave this profile? (y/n): ";
				cin >> saveChoice;

				if (tolower(saveChoice) == 'y') {
					Sweep_Profile newProfile(pName);
					for (const auto& instr : tempSessionInstr) {
						newProfile.addInstr(instr);
					}
					newProfile.saveProfile(true);
				}
				else {
					cout << "Configuration cancelled. Profile not saved.\n";
				}

				state = 0;
				task_exit();
				break;
		    }
			case 3: // LOAD PROFILE
			{
				cout << "\nLOAD PROFILE selected\n";

				// 1. Display Available Files
				std::vector<std::string> profileNames = getAvailableProfiles();
				if (profileNames.empty()) {
					cout << "[!] No profiles found in the 'Profiles' folder.\n";
					state = 0; break;
				}

				cout << "\n--- Available Profiles ---" << endl;
				for (size_t i = 0; i < profileNames.size(); ++i) {
					cout << i + 1 << ". " << profileNames[i] << endl;
				}

				// 2. Prompt for Selection
				int choice;
				cout << "\nSelect a profile number to load (or 0 to cancel): ";
				cin >> choice;

				if (choice <= 0 || choice > (int)profileNames.size()) {
					cout << "Selection cancelled.\n";
					state = 0;
					break;
				}

				string selectedName = profileNames[choice - 1];

				// --- MISSING STEP: Load the data from the file first! ---
				// This defines loadedSteps so the compiler knows what it is.
				vector<Instruction> loadedSteps = currentProfile.loadProfile(selectedName);

				if (loadedSteps.empty()) {
					cout << "[!] Error: Profile file '" << selectedName << "' was empty or could not be read.\n";
					state = 0;
					break;
				}

				cout << "\n--- PREVIEWING PROFILE: " << selectedName << " ---" << endl;
				cout << "Step | Pan    | Tilt   | Speed | Delay" << endl;
				cout << "---------------------------------------" << endl;

				for (size_t i = 0; i < loadedSteps.size(); ++i) {
					// Formatted output to keep columns aligned
					printf(" %2zu  | %5d  | %5d  | %5d | %5d\n",
						i + 1,
						loadedSteps[i].getPan(),
						loadedSteps[i].getTilt(),
						loadedSteps[i].getSpeed(),
						loadedSteps[i].getDelay());
				}

				char confirm;
				cout << "\nLoad these instructions onto devices? (y/n): ";
				cin >> confirm;

				if (tolower(confirm) != 'y') {
					cout << "Load cancelled by user." << endl;
					state = 0; break;
				}

				// 3. Select Target Devices
				printValidDevices();
				cout << "\nEnter device name(s) to apply this profile to, separated by commas:\n";
				string input;
				getline(cin >> ws, input);
				vector<string> names = processStrToArr(input);

				int count = 0;
				for (const string& name : names) {
					for (auto& dev : devices) {
						if (dev.getName() == name) {
							// Now loadedSteps is defined and contains the CSV data
							dev.loadInstructions(selectedName, loadedSteps);
							dev.setPlaying(false); // Staged, but not yet playing
							count++;
						}
					}
				}

				if (count == 0) {
					cout << "[!] No matching devices found." << endl;
				}
				else {
					cout << "[SUCCESS] Profile '" << selectedName
						<< "' staged on " << count << " device(s). Ready to Play." << endl;
				}

				state = 0;
				task_exit();
				break;
			}
		    case 4:
			{
				cout << "\nDELETE PROFILE selected" << endl;

				std::vector<std::string> profileNames = getAvailableProfiles();
				if (profileNames.empty()) {
					cout << "[!] No profiles found to delete." << endl;
					state = 0; break;
				}

				cout << "\n--- Select Profile to DELETE ---" << endl;
				for (size_t i = 0; i < profileNames.size(); ++i) {
					cout << i + 1 << ". " << profileNames[i] << endl;
				}

				int choice; 
				cout << "\nSelect a profile number (or 0 to cancel): ";
				cin >> choice;

				if (choice <= 0 || choice > (int)profileNames.size()) {
					cout << "Action cancelled." << endl;
					state = 0; break;
				}

				string selectedName = profileNames[choice - 1]; 
				// Preview before deletion
				vector<Instruction> loadedSteps = currentProfile.loadProfile(selectedName);

				cout << "\n--- REVIEWING CONTENTS OF: " << selectedName << " ---" << endl;
				if (!loadedSteps.empty()) {
					cout << "Step | Pan    | Tilt   | Speed | Delay" << endl;
					cout << "---------------------------------------" << endl;
					for (size_t i = 0; i < loadedSteps.size(); ++i) {
						printf(" %2zu  | %5d  | %5d  | %5d | %5d\n",
							i + 1,
							loadedSteps[i].getPan(),
							loadedSteps[i].getTilt(),
							loadedSteps[i].getSpeed(),
							loadedSteps[i].getDelay());
					}
				}

				char confirm;
				cout << "\n[WARNING] PERMANENTLY delete '" << selectedName << "'? (y/n): ";
				cin >> confirm;

				if (tolower(confirm) == 'y') {
					string filePath = "Profiles/" + selectedName + ".csv";
					if (fs::remove(filePath)) {
						cout << "[SUCCESS] Profile deleted." << endl;

						for (auto& dev : devices) {
							if (dev.getLoadedProfile() == selectedName) {
								dev.setLoadedProfile("None");
							}
						}
					}
					else {
						cout << "[ERROR] File not found." << endl;
					}
				}

				state = 0;
				task_exit();
				break;
			}
		    case 5:
		    {
			    cout << "\nPLAY PROFILE selected";
			    state = 0;
				task_exit();
			    break;
		    }
		    case 6:
		    {
			    cout << "\nSTOP PROFILE selected";
			    state = 0;
				task_exit();
			    break;
		    }
		    case 7:
		    {
			    cout << "\nACCESS LOG selected";
			    state = 0;
				task_exit();
			    break;
		    }
		    case 8:
		    {
			    cout << "\nDELETE LOG selected";
			    state = 0;
				task_exit();
			    break;
		    }
		    
		    default:
		    {
		        state = 0;
		        break;
		    }

		}
	}
}

void task_exit()
{
	cout << "\n\nTask Complete, returning to Main Menu. Hit Ctrl + C to exit the program\n";
}

bool enter_device_names(const vector<PanTiltDevice>& devices, const vector<string>& names)
{
	bool allValid = true;

	for (const auto& name : names)
	{
		bool found = false;

		for (const auto& dev : devices)
		{
			if (name == dev.getName())
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			cout << "Error: '" << name << "' is not valid.\n";
			allValid = false;
		}
	}

	return allValid;
}

vector<string> processStrToArr(const string& input)
{
	vector<string> result;
	stringstream ss(input);
	string token;

	while (getline(ss, token, ','))
	{
		// trim leading spaces
		size_t start = token.find_first_not_of(" ");
		if (start != string::npos)
			token = token.substr(start);

		// trim trailing spaces
		size_t end = token.find_last_not_of(" ");
		if (end != string::npos)
			token = token.substr(0, end + 1);

		if (!token.empty())
			result.push_back(token);
	}

	return result;
}

//Need Function to scan for available profiles
std::vector<std::string> getAvailableProfiles() {
	std::vector<std::string> profiles;
	std::string path = "Profiles";

	if (!fs::exists(path)) return profiles;

	for (const auto& entry : fs::directory_iterator(path)) {
		if (entry.path().extension() == ".csv") {
			// Get just the filename without the .csv extension
			profiles.push_back(entry.path().stem().string());
		}
	}
	return profiles;
}

void printValidDevices() {
	cout << "Valid devices:\n";
	for (const auto& dev : devices)
		cout << "- " << dev.getName() << "\n";
}


#include "Sweep_History.h"
#include "Sweep_Profile.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <sstream> 
#include <filesystem>
#include <thread> // Added for threading support

namespace fs = std::filesystem;

using namespace std;

// Function Prototypes
bool enter_device_names(const vector<PanTiltDevice*>& devices, const vector<string>& names);
vector<string> processStrToArr(const string& input);
void task_exit();
std::vector<std::string> getAvailableProfiles();
void printValidDevices();
void runDeviceSweep(PanTiltDevice* dev, Sweep_History* history, int logInterval);

// Global Variables
Sweep_Profile currentProfile("None");
std::vector<PanTiltDevice*> activeDevices;

// Hard define devices - Switched to Pointers to fix C2280 'deleted function' error
vector<PanTiltDevice*> devices =
{
	new PanTiltDevice("dev1", "001"),
	new PanTiltDevice("dev2", "002", 5, 5, 10),
	new PanTiltDevice("dev3", "003", 18),
	new PanTiltDevice("dev4", "004", 28, 15, 19)
};


int main()
{
	int state = 0;
	int choice = -1;


	for (const auto* device : devices)
	{
		std::cout << "---------------------\n";
		std::vector<std::string> status = device->getStatus();

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

					for (const auto* dev : devices)
					{
						if (name == dev->getName())
						{
							selectedDevices.push_back(dev);
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
				for (auto* dev : devices) {
					if (dev->getName() == name) {
						dev->loadInstructions(selectedName, loadedSteps);
						dev->setPlaying(false);
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

					for (auto* dev : devices) {
						if (dev->getLoadedProfile() == selectedName) {
							dev->setLoadedProfile("None");
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
			printValidDevices();

			cout << "\nEnter device name(s) to start, separated by commas:\n";
			string input;
			getline(cin >> ws, input);
			vector<string> names = processStrToArr(input);

			vector<PanTiltDevice*> selectedForPlay;

			// 1. Validation & Pre-Check
			for (const string& name : names) {
				bool found = false;
				for (auto* dev : devices) {
					if (dev->getName() == name) {
						found = true;
						if (dev->getLoadedProfile() == "None") {
							cout << "[!] Skipping " << name << ": No profile loaded." << endl;
						}
						else if (dev->isCurrentlyPlaying()) {
							cout << "[!] Skipping " << name << ": Device is already active." << endl;
						}
						else {
							selectedForPlay.push_back(dev);
						}
					}
				}
				if (!found) cout << "[!] Device '" << name << "' not found." << endl;
			}

			if (selectedForPlay.empty()) {
				cout << "[!] No valid devices to start. Aborting." << endl;
				state = 0; break;
			}

			// 2. Logging Configuration
			bool enableLogging = false;
			Sweep_History* history = nullptr;
			int logInterval = 0;

			cout << "\nWould you like to log the results? (y/n): ";
			char logChoice;
			cin >> logChoice;

			if (tolower(logChoice) == 'y') {
				enableLogging = true;
				string logFileName;
				cout << "Enter the name of the log file: ";
				cin >> logFileName;

				history = new Sweep_History(logFileName);

				cout << "Enter logging interval (every x seconds): ";
				cin >> logInterval;
			}

			// 3. Start Confirmation
			cout << "\nConfirm: Start sweep for " << selectedForPlay.size() << " device(s)? (y/n): ";
			char confirm;
			cin >> confirm;

			if (tolower(confirm) == 'y') {
				// 4. Thread Dispatch
				for (auto* dev : selectedForPlay) {
					dev->setPlaying(true);
					std::thread backgroundSweep(runDeviceSweep, dev, history, logInterval);
					backgroundSweep.detach();
				}
				cout << "\n[SYSTEM] Sweeps started. You can continue using the menu." << endl;
			}
			else {
				cout << "Sweep cancelled." << endl;
				if (history) delete history;
			}
			state = 0;
			task_exit();
			break;
		}
		case 6: // STOP SWEEP
		{
			cout << "\nSTOP SWEEP selected" << endl;

			bool anyActive = false;
			for (const auto* dev : devices) {
				if (dev->isCurrentlyPlaying()) {
					cout << " - " << dev->getName() << " (Active)" << endl;
					anyActive = true;
				}
			}

			if (!anyActive) {
				cout << "[!] No devices are currently performing a sweep." << endl;
				state = 0; break;
			}

			cout << "\nEnter device name(s) to stop, separated by commas: ";
			string input;
			getline(cin >> ws, input);
			vector<string> names = processStrToArr(input);

			int stoppedCount = 0;
			for (const string& name : names) {
				for (auto* dev : devices) {
					if (dev->getName() == name && dev->isCurrentlyPlaying()) {
						dev->requestStop(true);
						stoppedCount++;
					}
				}
			}

			if (stoppedCount > 0) {
				cout << "[SYSTEM] Stop signal sent to " << stoppedCount << " device(s)." << endl;
			}
			else {
				cout << "[!] No matching active devices found." << endl;
			}

			state = 0;
			task_exit();
			break;
		}
		case 7:
		{
			cout << "\nACCESS LOG selected";
			// 1. Scan for available logs
			string logPath = "Logs";
			if (!fs::exists(logPath)) {
				cout << "[!] No logs folder found." << endl;
				state = 0; break;
			}

			vector<string> logFiles;
			for (const auto& entry : fs::directory_iterator(logPath)) {
				if (entry.path().extension() == ".csv") {
					logFiles.push_back(entry.path().filename().string());
				}
			}

			if (logFiles.empty()) {
				cout << "[!] No log files found." << endl;
				state = 0; break;
			}

			cout << "\n--- Available Logs ---" << endl;
			for (size_t i = 0; i < logFiles.size(); ++i) {
				cout << i + 1 << ". " << logFiles[i] << endl;
			}

			int choice;
			cout << "\nSelect a log number to preview (or 0 to cancel): ";
			cin >> choice;

			if (choice <= 0 || choice > (int)logFiles.size()) {
				state = 0; break;
			}

			string selectedFile = logPath + "/" + logFiles[choice - 1];
			ifstream file(selectedFile);
			string line;
			vector<string> allLines;

			if (file.is_open()) {
				while (getline(file, line)) {
					allLines.push_back(line);
				}
				file.close();
			}

			if (allLines.empty()) {
				cout << "[!] Log file is empty." << endl;
			}
			else {
				cout << "\n--- Table Preview: " << logFiles[choice - 1] << " ---" << endl;

				// Define Column Widths (Adjust these based on your data length)
				const int wT = 22; // Timestamp
				const int wN = 10; // Name
				const int wI = 8;  // ID
				const int wP = 8;  // Pan
				const int wTi = 8; // Tilt
				const int wS = 8;  // Speed

				auto printSeparator = [&]() {
					cout << "+" << string(wT + 2, '-') << "+" << string(wN + 2, '-') << "+" << string(wI + 2, '-')
						<< "+" << string(wP + 2, '-') << "+" << string(wTi + 2, '-') << "+" << string(wS + 2, '-') << "+" << endl;
					};

				auto printRow = [&](const string& rowStr) {
					stringstream ss(rowStr);
					string t, n, id, p, ti, s;
					getline(ss, t, ','); getline(ss, n, ','); getline(ss, id, ',');
					getline(ss, p, ','); getline(ss, ti, ','); getline(ss, s, ',');

					cout << "| " << left << setw(wT) << t << " | " << setw(wN) << n << " | " << setw(wI) << id
						<< " | " << setw(wP) << p << " | " << setw(wTi) << ti << " | " << setw(wS) << s << " |" << endl;
					};

				// Header
				printSeparator();
				printRow("Timestamp,Name,ID,Pan,Tilt,Speed");
				printSeparator();

				int total = allLines.size();
				// Skip header in data if it exists (allLines[0])
				int startIdx = (allLines[0].find("Timestamp") != string::npos) ? 1 : 0;

				if (total <= 22) {
					for (int i = startIdx; i < total; ++i) printRow(allLines[i]);
				}
				else {
					// First 10 data lines
					for (int i = startIdx; i < startIdx + 10; ++i) printRow(allLines[i]);

					// Visual break
					cout << "| " << string(68, '.') << " |" << endl;

					// Last 10 lines
					for (int i = total - 10; i < total; ++i) printRow(allLines[i]);
				}
				printSeparator();
				cout << " Total Data Points: " << (total - startIdx) << endl;
			}
			state = 0;
			task_exit();
			break;
		}
		case 8:
		{
			cout << "\nDELETE LOG selected";
			string logPath = "Logs";
			if (!fs::exists(logPath)) {
				cout << "[!] No logs folder found." << endl;
				state = 0; break;
			}

			// 1. Scan for available logs
			vector<string> logFiles;
			for (const auto& entry : fs::directory_iterator(logPath)) {
				if (entry.path().extension() == ".csv") {
					logFiles.push_back(entry.path().filename().string());
				}
			}

			if (logFiles.empty()) {
				cout << "[!] No log files found to delete." << endl;
				state = 0; break;
			}

			// 2. Display List
			cout << "\n--- Select Log to DELETE ---" << endl;
			for (size_t i = 0; i < logFiles.size(); ++i) {
				cout << i + 1 << ". " << logFiles[i] << endl;
			}

			// 3. Prompt for selection
			int choice;
			cout << "\nSelect a log number (or 0 to cancel): ";
			cin >> choice;

			if (choice <= 0 || choice > (int)logFiles.size()) {
				cout << "Action cancelled." << endl;
				state = 0; break;
			}

			string selectedLog = logFiles[choice - 1];

			// 4. Final Confirmation
			char confirm;
			cout << "\n[WARNING] Are you sure you want to PERMANENTLY delete '" << selectedLog << "'? (y/n): ";
			cin >> confirm;

			if (tolower(confirm) == 'y') {
				string fullPath = logPath + "/" + selectedLog;
				if (fs::remove(fullPath)) {
					cout << "[SUCCESS] Log file '" << selectedLog << "' has been deleted." << endl;
				}
				else {
					cout << "[ERROR] Could not delete the file. It may be in use by another process." << endl;
				}
			}
			else {
				cout << "Deletion aborted." << endl;
			}

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

bool enter_device_names(const vector<PanTiltDevice*>& devices, const vector<string>& names)
{
	bool allValid = true;

	for (const auto& name : names)
	{
		bool found = false;

		for (const auto* dev : devices)
		{
			if (name == dev->getName())
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
		size_t start = token.find_first_not_of(" ");
		if (start != string::npos)
			token = token.substr(start);

		size_t end = token.find_last_not_of(" ");
		if (end != string::npos)
			token = token.substr(0, end + 1);

		if (!token.empty())
			result.push_back(token);
	}

	return result;
}

std::vector<std::string> getAvailableProfiles() {
	std::vector<std::string> profiles;
	std::string path = "Profiles";

	if (!fs::exists(path)) return profiles;

	for (const auto& entry : fs::directory_iterator(path)) {
		if (entry.path().extension() == ".csv") {
			profiles.push_back(entry.path().stem().string());
		}
	}
	return profiles;
}

void printValidDevices() {
	cout << "Valid devices:\n";
	for (const auto* dev : devices)
		cout << "- " << dev->getName() << "\n";
}

void runDeviceSweep(PanTiltDevice* dev, Sweep_History* history, int logInterval) {
	const std::vector<Instruction>& steps = dev->getActiveInstructions();
	auto lastLogTime = std::chrono::steady_clock::now();

	dev->requestStop(false);

	if (history) {
		history->addEvent(Event(dev->getName(), dev->getID(), dev->getPanPos(), dev->getTiltPos(), dev->getSpeed()));
	}

	for (const auto& instr : steps) {

		if (dev->isStopRequested()) {
			break;
		}

		dev->setPanPos(instr.getPan());
		dev->setTiltPos(instr.getTilt());
		dev->setSpeed(instr.getSpeed());

		int remainingDelay = instr.getDelay();
		while (remainingDelay > 0) {

			if (dev->isStopRequested()) {
				break;
			}

			if (history) {
				auto now = std::chrono::steady_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastLogTime).count();
				if (elapsed >= logInterval) {
					history->addEvent(Event(dev->getName(), dev->getID(), dev->getPanPos(), dev->getTiltPos(), dev->getSpeed()));
					lastLogTime = now;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			remainingDelay -= 100;
		}

		if (dev->isStopRequested()) {
			break;
		}
	}

	if (history) {
		history->addEvent(Event(dev->getName(), dev->getID(), dev->getPanPos(), dev->getTiltPos(), dev->getSpeed()));
	}

	dev->setPlaying(false);
	dev->requestStop(false);

	std::cout << "\n[NOTIFY] " << dev->getName() << " sweep finished or stopped.\n";
}
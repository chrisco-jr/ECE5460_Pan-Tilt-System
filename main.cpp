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

using namespace std;

// Function Prototypes
bool enter_device_names(const vector<PanTiltDevice>& devices, const vector<string>& names);
vector<string> processStrToArr(const string& input);
void task_exit();

int main()
{
	int state = 0;
	int choice = -1;
	
	// Hard define devices for sake of demo

	vector<PanTiltDevice> devices =
	{
		PanTiltDevice("dev1", "001"),
		PanTiltDevice("dev2", "002", 5, 5, 10),
		PanTiltDevice("dev3", "003", 18),
		PanTiltDevice("dev4", "004", 28, 15, 19)
	};

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
			    cout << "\nSTATUS selected";
				string input;
				bool allValid = false;

				while (!allValid)
				{
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
						cout << "Valid devices:\n";
						for (const auto& dev : devices)
							cout << "- " << dev.getName() << "\n";
					}
				}

			    state = 0;
				task_exit();
			    break;
		    }
		    case 2:
		    {
			    cout << "\nCONFIGURE PROFILE selected";
				cout << "\nCreating new Sweep Profile...";
				cout << "\nEnter profile name: ";
				string profileName;
				cin >> profileName;

				bool moreInstr = true;
				Sweep_Profile newProfile(profileName);

			    state = 0;
				task_exit();
			    break;
		    }
		    case 3:
		    {
			    cout << "\nLOAD PROFILE selected";
			    state = 0;
				task_exit();
			    break;
		    }
		    case 4:
		    {
			    cout << "\nDELETE PROFILE selected";
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

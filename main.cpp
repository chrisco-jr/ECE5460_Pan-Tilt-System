#include "Event.h"
#include "Instruction.h"
#include "Pan_Tile Device.h"
#include "Sweep_History.h"
#include "Sweep_Profile.h"
#include <iostream>
#include <string.h>
#include <limits>
#include <stdlib.h>

using namespace std;

// Function Prototypes
char** processStrToArr(const char *input, int *count);
bool enter_device_names(char *input);
void task_exit();

const char *DEVICES[] = {"dev1", "dev2", "dev3", "dev4"};
	int DEVICE_COUNT = sizeof(DEVICES) / sizeof(DEVICES[0]);

int main()
{
	int state = 0;
	int choice = -1;
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
				string devices;
				bool allValid = false;
				while (!allValid)
				{
					cout << "\nEnter device name(s) to see status of, separated by a comma\n";
					getline(cin >> ws, devices);
					if(enter_device_names((char*)devices.c_str()))
					{	
						cout << "All devices valid";
						allValid = true;
						break;
					}
					else
					{
						cout << "Please try again.\n";
					}
				}
			    state = 0;
				task_exit();
			    break;
		    }
		    case 2:
		    {
			    cout << "\nCONFIGURE PROFILE selected";
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

bool enter_device_names(char *input)
{
	int count = 0;
	char** deviceNames = processStrToArr(input, &count);

	if (deviceNames == NULL && count == 0)
	{
		cout << "No devices entered. Valid devices:\n";
		for (int i = 0; i < DEVICE_COUNT; i++)
		{
			printf("- %s\n", DEVICES[i]);
		}
		return false;
	}

	bool allValid = true;

	for (int i = 0; i < count; i++)
	{
		bool isValid = false;
		for (int j = 0; j < DEVICE_COUNT; j++)
		{
			if (strcmp(deviceNames[i], DEVICES[j]) == 0) {
                isValid = true;
                break;
            }
		}
		if (!isValid)
		{
			allValid = false;
			printf("Error: '%s' is not a valid device.\n", deviceNames[i]);
			cout << "Valid devices:\n";
			for (int i = 0; i < DEVICE_COUNT; i++)
			{
				printf("- %s\n", DEVICES[i]);
			}
		}
	}

	for (int i = 0; i < count; i++)
	{
		free(deviceNames[i]);
	}
	free(deviceNames);
	return allValid;
}

char** processStrToArr(const char *input, int *count)
{
	extern const char* DEVICES[];
    extern int DEVICE_COUNT;
	
	// processes a list of strings seprated by commas, returns as an array
	char **result = NULL;
	char *tmpInput = strdup(input);
	char *token = strtok(tmpInput, ",");
	*count = 0;

	while (token != NULL)
	{
		while (*token == ' ') token++;
		char **tmpResult = (char**)realloc(result, sizeof(char*) * (*count + 1));
		if (tmpResult == NULL)
		{
			free(tmpInput);
			return result;
		}
		result = tmpResult;
		result[*count]=(char*)strdup(token);
		(*count)++;

		token = strtok(NULL, ",");
	}
	free(tmpInput);
	return result;
}
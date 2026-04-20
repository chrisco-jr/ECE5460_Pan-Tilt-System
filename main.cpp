#include "Event.h"
#include "Instruction.h"
#include "Pan_Tile Device.h"
#include "Sweep_History.h"
#include "Sweep_Profile.h"
#include <iostream>
#include <limits>

using namespace std;
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
			    state = 0;
			    break;
		    }
		    case 2:
		    {
			    cout << "\nCONFIGURE PROFILE selected";
			    state = 0;
			    break;
		    }
		    case 3:
		    {
			    cout << "\nLOAD PROFILE selected";
			    state = 0;
			    break;
		    }
		    case 4:
		    {
			    cout << "\nDELETE PROFILE selected";
			    state = 0;
			    break;
		    }
		    case 5:
		    {
			    cout << "\nPLAY PROFILE selected";
			    state = 0;
			    break;
		    }
		    case 6:
		    {
			    cout << "\nSTOP PROFILE selected";
			    state = 0;
			    break;
		    }
		    case 7:
		    {
			    cout << "\nACCESS LOG selected";
			    state = 0;
			    break;
		    }
		    case 8:
		    {
			    cout << "\nDELETE LOG selected";
			    state = 0;
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
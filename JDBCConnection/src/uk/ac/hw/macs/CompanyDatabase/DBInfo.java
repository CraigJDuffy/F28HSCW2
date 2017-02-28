package uk.ac.hw.macs.CompanyDatabase;


/*
GUI DBMS Application 
Monica Farrow 

Contains all details about the database :
	Database Name
	OwnerName
	Table / View names
	Descriptions and corresponding table/view names

Methods;
	For descriptions, returns DescQuery object, given the description

*/

class DBInfo
{
//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++
//Just change from here to the next dividing line

	public static String dbName = "Sports Centre";		//database subject - free choice
	public static String tables [] =
		{							//table / view  names that don't need a description
									//i.e. the name is self-explanatory
			"",					    //Leave first one blank
			"Booking", 
			"Equipment", 
			"Member",
			"Paygrade",
			"Room",
			"Staff",
		    "Training",
		 };
	
	//Table/ View names and Description
	//i.e. tables or views where a description is useful
	public static  DescQuery descQueries [] = 
		{
	        new DescQuery ("", ""),  //LEAVE BLANK
		    new DescQuery ("Has", "Rooms and the equipment in them"),
		    new DescQuery("HasPassed", "Member ID numbers and the training they've passed"),
		    new DescQuery ("Membership", "List of membership levels offered"),
		    new DescQuery("Requires", "Equipment and the training they require"),
		    new DescQuery("Uses", "The equipment used by each booking"),
		    new DescQuery("ViewEquipmentTraining", "Shows all details of equipment with associated training requirements"),
		    new DescQuery("ViewFreeEquipment", "All equipment which require no training"),
		    new DescQuery("ViewMemberTraining", "All details of training passed by member"),
		    new DescQuery("ViewMemberEquipment", "All mIDs and equipment they're trained on"),
		    new DescQuery("ViewStaffMembers", "Show details of Staff Members from Member table"),
		    new DescQuery("ViewNonStaffMembers", "Show all details of non-staff members"),
		    new DescQuery("ViewRoomEquipment", "Room details with equipment they hold")
		};
	//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++//++
	//Don't change anything beyond this line
	
	//Find desc query object from list, using query description
	public static DescQuery findDescQuery(String queryTitle)  
	{
		DescQuery q = descQueries[0];
		for (int i = 0; i < descQueries.length; i++)
		{
			if (queryTitle.equals(descQueries[i].getDesc()))
			{
				q = descQueries[i];
			}
		}
		return q;	
	}	
}
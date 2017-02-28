package uk.ac.hw.macs.CompanyDatabase;


import java.sql.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class JDBCMySQLApp extends JFrame 
						implements ActionListener
{
	//CONNECTING TO MySQL
	//declare driver
	final String driver = "com.mysql.jdbc.Driver";
	//declare the fixed part of the url
	final String urlstart = "jdbc:mysql://mysql-server-1/";
	String urluserpass;
	
	//database connection	
	Connection con;
	
	//swing components
	JScrollPane scroll;
	JTable tab;
	JButton parQuery,ownSQL, ownQueries;
	JComboBox tableList, descList;
	JLabel tableHeading;
	QueryTable  quTable;
	JTextArea feedback;
	
	//first part of SQL to list complete table
	String selectAll = "Select * from ";
	
	//instructions for switchboard
	String tableListLabel = "Choose a name to see the contents of that table or view ";
	String descListLabel = "Choose a description to see the details of table or view ";
	String ownSQLLabel = "Enter your own SQL query ";
	String interactiveLabel = "Choose from a set of queries";
		
	//connects to database and sets up GUI 
	public JDBCMySQLApp() 
	{
		//initialises frame
		super();
		this.setTitle(DBInfo.dbName + " SwitchBoard");        
		this.setSize(750, 650);
		this.addWindowListener(new WindowCloser());
		
		//connects to database
  		connectToDatabase();
		//MAKE  TOP PANEL
		JPanel topPanel = new JPanel();			
		topPanel.setLayout(new GridLayout(5,3));
		
  		// Add each table to the table combo box 
		tableList = new JComboBox();
	    tableList.setEditable(true);
		for (int i = 0; i < DBInfo.tables.length; i++)
		{
			tableList.addItem(DBInfo.tables[i]);
		}
		tableList.addActionListener(this);
		topPanel.add(new JLabel(tableListLabel, SwingConstants.RIGHT));
		topPanel.add(tableList);
		
		
		// Add each description and view to the query combo box
		descList = new JComboBox();
	    descList.setEditable(true);

		for (int i = 0; i < DBInfo.descQueries.length; i++)
		{
			descList.addItem(DBInfo.descQueries[i].getDesc());
		}
	   	descList.addActionListener(this); 
	   	topPanel.add(new JLabel(descListLabel, SwingConstants.RIGHT));
	   	topPanel.add(descList);
		
		
		//add own queries button
		ownQueries = new JButton("Set of queries");
		ownQueries .addActionListener(this);
		topPanel.add(new JLabel("Set of queries", SwingConstants.RIGHT));
		topPanel.add(ownQueries);
		
		//add own SQL Query button
		ownSQL = new JButton("New SQL Query");
		ownSQL.addActionListener(this);
		topPanel.add(new JLabel(ownSQLLabel, SwingConstants.RIGHT));
		topPanel.add(ownSQL);
		
		
		//add table heading
		tableHeading = new JLabel("");	
		tableHeading.setForeground(Color.black);
		tableHeading.setFont(new Font("SansSerif", Font.BOLD, 16));	
		topPanel.add(tableHeading);
		
		//addTextArea
		JPanel sPanel = new JPanel();
		feedback = new JTextArea(5,60);
		JScrollPane jsp = new JScrollPane(feedback);
		sPanel.add(jsp);
		

		//CREATE TABLE IN SCROLLPANE		
		//The table will contain the query results
		quTable = new QueryTable();
		tab = new JTable(quTable);
		scroll = new JScrollPane(tab);

		
		//ADD PANELS TO CONTENT PANE
		getContentPane().add(topPanel, BorderLayout.NORTH);
		getContentPane().add(sPanel, BorderLayout.SOUTH);
		getContentPane().add(scroll, BorderLayout.CENTER);
		
    }
    
    
    /////////////////////////////////////////////////////////////////
    //METHOD actionPerformed
    //Handles all actions from GUI
    //If table or view listing required, execute 'Select All'
    //	(table or view is displayed in scrollpane)
    //Else If set of queries or own SQL query is required
    //	open specialised frame
	public void actionPerformed(ActionEvent evt)  
	{
		Object source = evt.getSource();
		
		//If table/view listing required, execute 'Select All'
		// (table is displayed in scrollpane)
		if (source == tableList)
		{
			String sql ="";
			try
			{
				String table = (String) tableList.getSelectedItem();
				if (!table.equals("")) {
					Statement stmt = con.createStatement();
					sql = selectAll + table;
					ResultSet rs= stmt.executeQuery(sql);
					quTable.formatTable(rs);

				}
				else {
					quTable.clearTable();
				}
				tableHeading.setText(table);
				feedback.setText("");
			}
			catch (SQLException e)
			{
				feedback.setText(e + sql);
			}
		}
		
		//If description listing required, find table/view name, 
		// then execute 'Select All'
		// (table displayed in scrollpane)
		else if (source == descList)
		{
			String sql = "";
			try
			{
				String queryDesc = (String)descList.getSelectedItem();
				if (!queryDesc.equals("")) {
					DescQuery dq = DBInfo.findDescQuery(queryDesc);
					sql = selectAll +  dq.getName();
					Statement stmt = con.createStatement();
					ResultSet rs= stmt.executeQuery(sql);
					quTable.formatTable(rs);
					tableHeading.setText(dq.getDesc());	
					feedback.setText("");
				}
			}
			catch (SQLException e)
			{
				feedback.setText(e + sql);
			}		
		}
		
		else if (source == ownQueries) {
			feedback.setText("");
			OwnQueries pq = new OwnQueries(con);
			pq.setVisible(true);
		}
		// if own SQL Query required, open SQL frame
		else if (source == ownSQL)
		{
			feedback.setText("");
			SQLQuery sq = new SQLQuery(con);
			sq.setVisible(true);
		}

	}
	
	
	/////////////////////////////////////////////////////////////////
	// Obtain user name and password	 
	// Connect to database 	
	void connectToDatabase()
	{
		// Obtain user name and password	
		
		UserPassDialog upd = new UserPassDialog(this);
		upd.setLocation(200,200);
		LogOn logonInfo = upd.showDialog();		
		String username = logonInfo.getUName();
		String password = logonInfo.getPWord();
		String dbname = username;
		
		String connectDetails = urlstart + dbname + "?user=" + username + "&password=" 
                + password; 

		// Connect to database 
		try 
		{
			//connect to the database
            Class.forName(driver).newInstance();
            con = DriverManager.getConnection(connectDetails);
		}
		catch(Exception e) 
		{
  			System.out.println("Could not initialize the database." 
											+ e.getMessage());
			e.printStackTrace();
  			System.exit(0);
		}

   	}
   	


   	/////////////////////////////////////////////////////////////////
   	public static void main(String args[]) 
  	{		
  		JDBCMySQLApp f = new JDBCMySQLApp();
		f.setVisible(true);	
	}
	  
	  

	/////////////////////////////////////////////////////////////////  	
	// Inner class for window closing 
	// Close DB conection then exit
	class WindowCloser extends WindowAdapter
	{
		public void windowClosing(WindowEvent event)
		{	
  			try 
  			{
				if (con != null) 		
				{  	
					con.close();
	    		}
				System.exit(0);
	  		}
  	
  			catch(Exception e) 
  			{
    			System.out.println("Could not close the current connection.");
    			e.printStackTrace();
	  		}
		
		}
	}
	
}
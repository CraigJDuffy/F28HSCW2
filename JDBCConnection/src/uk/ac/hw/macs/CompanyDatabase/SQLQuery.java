package uk.ac.hw.macs.CompanyDatabase;

 /*
class SQLQuery
DBMS Application
Monica Farrow / Calum Hutchison
A window for entry of SQL Query, with table for results  
 
*/
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.sql.*;

public class SQLQuery extends JFrame 
                      implements ActionListener
{   
  	JTextArea queryField;
  	QueryTable tableResults;
	Connection db;
	JButton execute, update;
  
  	/////////////////////////////////////////////////////////////////
  	//Constructor sets up frame with 
  	//	an area for SQL entry 
  	//	a table for results
  	public SQLQuery(Connection conn) 
	{
		//Initialise frame
	  	super("SQL Query");
	   	addWindowListener(new WindowAdapter()
							{public void windowClosing (WindowEvent e)	
								{dispose();}});
	   	setSize(650, 600);
	   	
		db = conn;
	    //Create upper panels for label, text area and buttons
	    JPanel p1 = new JPanel();
	    JPanel pn = new JPanel();
	   	p1.setLayout(new BorderLayout());
	    
	    //Add label 
	    pn.setLayout(new GridLayout (2,1));
	   	JLabel label1 = new JLabel("Enter your query here. "
								+ "Leave out final semi-colon "); 
	   	pn.add(label1);

	   	p1.add(pn, BorderLayout.NORTH);
	   	
		//Add execute button
	    execute = new JButton("Execute query");
	    execute .addActionListener(this) ;
	    JPanel buttonPanel = new JPanel();
	    buttonPanel.add(execute ); 
	    
		//Add update button
	    update = new JButton("Execute update");
	    update .addActionListener(this) ;
	    buttonPanel.add(update);
	    p1.add(buttonPanel, BorderLayout.SOUTH);
	    
	    //Add text area
	    queryField = new JTextArea("",8,50); 
	   	p1.add(queryField, BorderLayout.CENTER);   
	
		//Add panel to content pane
	    getContentPane().add(p1, BorderLayout.NORTH);
	    
	    //Set QueryTable in scrollpane in centre of layout
	   	//Results from the query will be stored here
	    tableResults = new QueryTable ();
	    JTable table = new JTable(tableResults);
	    JScrollPane scrollpane = new JScrollPane(table);
	    getContentPane().add(scrollpane, BorderLayout.CENTER);
  	}
  
  	/////////////////////////////////////////////////////////////////
  	//Handles all actions from the GUI
  	//Action can only be the Execute button
  	//Get query text from the text area
  	//Execute query (results displayed in table)
  	//Original query or error message displayed in text area
  	public void actionPerformed(ActionEvent e) 
  	{
		String sql = queryField.getText().trim();
		try
		{
			Statement stmt = db.createStatement();
			if (e.getSource() == execute) {
				ResultSet rs= stmt.executeQuery(sql);
				tableResults.formatTable(rs);
			}
			else {
				int rowsAffected = stmt.executeUpdate(sql);
				JOptionPane.showMessageDialog(this, 
						rowsAffected + " rows Affected");
			}
				
		}
		catch(SQLException ex)
		{
   			queryField.setText(ex + sql); 
		}    
  	}
	
}
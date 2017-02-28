package uk.ac.hw.macs.CompanyDatabase;

 

import java.sql.*;
import java.util.Vector;
import javax.swing.table.*;

public class QueryTable extends AbstractTableModel 
{
	
	Vector <String []> myTable;  			// will hold String[] for each row.
  	int colCount;
	ResultSet rs;
	ResultSetMetaData meta;

	/////////////////////////////////////////////////////////////////
	//Constructor stores connection and creates new table
  	public QueryTable()  
	{
		myTable = new Vector<String []> ();	
		colCount = 0;
		rs = null;
		meta = null;
 	}
  	
  	public void clearTable() {
  		myTable = new Vector<String []> ();
  		colCount = 0;
  		rs = null;
  		meta = null;
  		fireTableChanged(null); // notify new table.
  	}
	
   /////////////////////////////////////////////////////////////////
  	// getColumnName must be implemented for the AbstractTableModel
  	// to avoid column names A,B,C...
	public String getColumnName(int i) 
	{ 
		try
		{	
			if (meta == null)
				return "";
			else 
				return meta.getColumnName(i+1);
		}
		catch(SQLException e)
		{
			return "SQLException QT getColumnName" + e;
		}
	 }
	 
	// these 3 methods MUST be implemented for the AbstractTableModel
 	public int getColumnCount() { return colCount; }
 	
	public int getRowCount() { return myTable.size();}

	public Object getValueAt(int row, int col) 
	{ 
  		return ((String[])myTable.elementAt(row))[col];
	}


	////////////////////////////////////////////////////////////
	//For each record in the result set
	//	make an array of Strings  (each attribute)
	//	(This is adequate for displaying all fields except dates
	// 	 which are converted to the format DD MM YY )
	//Add record (array of Strings) to table vector	
	public void formatTable(ResultSet rs)
	{
		try
		{
			meta = rs.getMetaData();

			myTable = new Vector<String []> ();		
			colCount = meta.getColumnCount();
			
			//for each record, make array of strings
			//add to table
			while (rs.next())
			{
				String[] record = new String[colCount];
				for (int i=0; i < colCount; i++) 
				{
					record[i] = rs.getString(i+1);

					//If column is Date column, change format of string
					if(meta.getColumnTypeName(i+1).equals("DATE"))
					{	
				        if (record[i] != null) 
				        {
			                record[i] = record[i].substring(8,10) + "/" 
			                         + record[i].substring(5,7)  + "/" 
			                         + record[i].substring(0,4);
				        }
					}
				}	
				myTable.addElement(record);
			}

			fireTableChanged(null); // notify new table.
		}
		catch(SQLException e)
		{
			System.out.println("SQL X : QT formatTable");
			e.printStackTrace();
		}
	}
	


}
 

 	
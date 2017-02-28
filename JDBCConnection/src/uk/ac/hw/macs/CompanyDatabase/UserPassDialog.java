package uk.ac.hw.macs.CompanyDatabase;
/*
DBMS Application
Monica Farrow
(Adapted from a similar class in Core Java by Horstmann)
A dialog box to obtain type of database,username and password,
*/

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

class UserPassDialog extends JDialog
						implements ActionListener
{
	private JTextField username;
	private JPasswordField password;
	private JButton okButton;
	
	public UserPassDialog(JFrame parent)
	{
		super(parent, "Log on", true);
		setSize(240,160);
	
		//panel for username and password text fields
		JPanel userpassPanel = new JPanel();
		userpassPanel.setLayout(new GridLayout(2,2));
		
		userpassPanel.add(new JLabel("User name:"));
		userpassPanel.add(username = new JTextField(""));
		
		userpassPanel.add(new JLabel("Password:"));
		userpassPanel.add(password = new JPasswordField(""));
		password.setBackground(Color.lightGray);
		
		
		getContentPane().add("Center",userpassPanel);
		
		//Panel for button
		Panel pOK = new Panel();
		okButton = new JButton("OK");
		okButton.addActionListener(this);
		pOK.add(okButton);	
		getContentPane().add("South",pOK);
		

	}
	
	//come here when OK button clicked
	public void actionPerformed(ActionEvent e)
	{
		Object source = e.getSource();
		if (source == okButton)
		{
			setVisible(false);
		}

	}
	
	//show Dialog
	public LogOn showDialog()
	{
		this.setVisible(true);
		String un = username.getText();
		String pw = new String (password.getPassword());
		return new LogOn(un,pw);
	}
		
}
		
			

	
/*

  MusicXML Library
  Copyright (C) 2003  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  grame@grame.fr

*/

#ifndef __xml__
#define __xml__

#include "exports.h"
#include "bimap.h"
#include "smartpointer.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace MusicXML 
{

class xmlelement;
class xmlattribute;
class xmlable;
class xmlheader;

typedef SMARTP<xmlattribute> 	Sxmlattribute;
typedef SMARTP<xmlelement> 		Sxmlelement;
typedef SMARTP<xmlheader> 		Sxmlheader;

std::ostream& operator<< (std::ostream& os, const Sxmlattribute& attr);
EXP std::ostream& operator<< (std::ostream& os, const Sxmlelement& elt);
EXP std::ostream& operator<< (std::ostream& os, const Sxmlheader& header);

/*!
\internal
\brief to be used in place of std::endl
	to provide a correct indentation of the xml output.
	
	The object is defined as a singleton thus it will
	not operate correctly on parrallel formatting operations.
*/
class xmlendl {
	public:
		static xmlendl& get() { if (!fInstance) fInstance = new xmlendl; return *fInstance; }

		//! increase the indentation
		xmlendl& operator++ (int)  { fIndent++; return *this; }
		//! decrease the indentation
		xmlendl& operator-- (int)  { fIndent--; return *this; }
		//! reset the indentation to none
		void reset()               { fIndent = 0; }
		void print(std::ostream& os) const { int i = fIndent;
                                        os << std::endl;
                                        while (i-- > 0)  os << "    "; }
	protected:
		xmlendl() : fIndent(0) {}
	private:
		int fIndent;
		static xmlendl * fInstance;
};

/*!
\brief A generic xml attribute representation.

	An attribute is represented by its name and its value.
*/
class xmlattribute : public smartable {
    protected:
		xmlattribute(string name, string value) : fName(name), fValue(value) {}
		xmlattribute(string name, long value) : fName(name) { setValue(value); }
		xmlattribute(string name, float value) : fName(name) { setValue(value); }
    public:
		EXP friend SMARTP<xmlattribute> new_xmlattribute(string name, string value);
		EXP friend SMARTP<xmlattribute> new_xmlattribute(string name, long value);
		EXP friend SMARTP<xmlattribute> new_xmlattribute(string name, float value);
		EXP friend SMARTP<xmlattribute> new_xmlattribute(string name, int value);

		void setName (const string& name);
		void setValue (const string& value);
		void setValue (long value);
		void setValue (float value);

		const string& getName () const	{ return fName; }
		const string& getValue () const	{ return fValue; }
		void print (std::ostream& os) const;
    private:
		//! the attribute name
		string	fName;
		//! the attribute value
		string 	fValue;
};

/*!
\brief A generic xml element representation.

	An element is represented by its name, its value,
	the lists of its attributes and its enclosed elements.
	Attributes and elements must be added in the required 
	order.
*/
class xmlelement : public smartable {

	public:
        EXP friend SMARTP<xmlelement> new_xmlelement(string name);
        EXP friend SMARTP<xmlelement> new_xmlelement(string name, unsigned long val);
        EXP friend SMARTP<xmlelement> new_xmlelement(string name, long val);
        EXP friend SMARTP<xmlelement> new_xmlelement(string name, int val);
        EXP friend SMARTP<xmlelement> new_xmlelement(string name, float val);
        EXP friend SMARTP<xmlelement> new_xmlelement(string name, string val);

		void setValue (unsigned long value);
		void setValue (long value);
		void setValue (int value);
		void setValue (float value);
		void setValue (const string& value);
		void setName (const string& name);

		const string& getName () const				{ return fName; }
		const string& getValue () const				{ return fValue; }

		long add (const Sxmlattribute& attr);
		long add (const Sxmlelement& elt);
        
        const vector<Sxmlelement>& elements() const 		{ return fElements; }
        const vector<Sxmlattribute>& attributes() const 	{ return fAttributes; }
		
		bool empty () const { return fValue.empty() && fElements.empty(); }
		void print (std::ostream& os) const;

    protected:
		xmlelement(string name) : fName(name) {}
		xmlelement(string name, unsigned long value) : fName(name) { setValue(value); }
		xmlelement(string name, long value) : fName(name) { setValue(value); }
		xmlelement(string name, int value) : fName(name) { setValue(value); }
		xmlelement(string name, float value) : fName(name) { setValue(value); }
		xmlelement(string name, string value) : fName(name) { setValue(value); }
		virtual ~xmlelement() {}

    private:
		//! the element name
		string	fName;
		//! the element value
		string 	fValue;
		//! list of the element attributes
		vector<Sxmlattribute> fAttributes;
		//! list of the enclosed elements
		vector<Sxmlelement>   fElements;
};

/*!
\brief a class for generating the MusicXM header.
*/
class EXP xmlheader : public smartable {
     protected:
		xmlheader (int mode, string loc) : fMode(mode), fLocation(loc) {}
		virtual ~xmlheader () {}

     public:
		enum { partwise=1, timewise, last=timewise };
		
        EXP friend SMARTP<xmlheader> new_xmlheader(int mode=xmlheader::partwise, string loc="http://www.musicxml.org/dtds/");
	
		//! sets the MusicXML score mode
        void setMode (int mode);
		//! sets the MusicXML dtd location
        void setLocation (const string& loc);

		//! returns the MusicXML score mode
                int		getMode () const			{ return fMode; }
		//! returns the MusicXML dtd location
		const string&	getLocation () const		{ return fLocation; }
		//! returns the current supported MusicXML version
		const string	MusicXMLVersion () const 	{ return "1.0"; }
		void print (std::ostream& os) const;
        
	//! provides conversion from numeric to string modes
	static const string	scoreMode (int mode);
	//! provides conversion from string to numeric modes
	static int		scoreMode (string mode);

	private:
		//! the MusicXML score mode: between partwise and timewise
        int 	fMode;
		//! the MusicXML dtd location
		string 	fLocation;

	static bimap<string, int> fMode2String;
	static int 		fModeTbl[];
	static string 	fModeStrings[];
};

/*!
\brief an abstract class for xml able objects.

	xml able objects are objects that can be represented
	in xml. They must implement the \e xml() method which
	returns their xml description.
*/
/*
class xmlable {
     public:
		virtual Sxmlelement xml() const = 0;
};
*/

}

#endif

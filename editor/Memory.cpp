/** Minicomputer
 * industrial grade digital synthesizer
 * editorsoftware
 * Copyright 2007 Malte Steiner
 * This file is part of Minicomputer, which is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Minicomputer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <unistd.h>
#include <sys/stat.h>
#include "Memory.h"
/**
 * constructor
 */
Memory::Memory()
{
	int i;
	for (i=0;i<8;++i)
	{
		choice[i] = 0;
	}	
	//char zeichenkette[128];
	for ( i = 0;i<512;++i)
	{
		sprintf(sounds[i].name,"%i untitled",i);
		//strcpy(sounds[i].name,zeichenkette);
	}
	
	for ( i = 0;i<128;++i)
	{
		sprintf(multis[i].name,"%i untitled",i);
		//strcpy(multis[i].name,zeichenkette);
	}
	const char *home = getenv("HOME");
	if (!home)
		folder = ""; // ok, $HOME is not set so save it just HERE
	else
	{
		folder = std::string(home) + "/.miniComputer";
		mkdir(folder.c_str(), 0755);
	}
}
/**
 * destructor
 */
Memory::~Memory()
{
}

/**
 * retreive name of a certain patch
 * @param voicenumber of which we want to know the entry (should be redundant)
 * @param soundnumber
 * @return the name as string
 */
string Memory::getName(unsigned int voice,unsigned int Eintrag)
{
	return sounds[Eintrag].name;
}
/**
 * set the soundnumber to a certain voice
 * @param voice number
 * @param sound number
 */
void Memory::setChoice(unsigned int voice,unsigned int i)
{
	if ((i>=0) && (i<512)) // check if its in the range
	{
		choice[voice] = i;
	}
	else // oha, should not happen
	{
		printf("illegal sound choice\n");
		fflush(stdout);
	}
}
/**
 * return a sound id of a certain given voice number
 * @param the voice number
 * @return the sound number of that voice
 */
unsigned int Memory::getChoice(unsigned int voice)
{
	return choice[voice];
}
/**
 * save the complete soundmemory to disk
 * two fileformats are possible, the historical
 * but depricated binary format with is just a memory dump
 * and not extensible and the textformat, which takes more space
 * on harddisk but is human editable and extensible
 * the filenames are fix
 */
void Memory::save()
{
	    /*ofstream ofs("minicomputerMemory.mcm", std::ios::binary);
  //boost::archive::text_oarchive oa(ofs);
	for (int i=0;i<139;i++)
	{
	ofs<<sounds[0].parameter[i];
	//string name = sounds[0].name;
	//ofs.write((char *)&name, sizeof(name));
  //oa << sounds[i];
	}
  ofs.close();*/
// *************************************************************
// new fileoutput as textfile with a certain coding which is
// documented in the docs
	std::string temp = folder + "/minicomputerMemory.temp";

ofstream File (temp.c_str()); // temporary file
int p,j;
for (int i=0;i<512;++i)
 {  
	File<< "["<<i<<"]" <<endl;// write the soundnumber
	File<< "'"<<sounds[i].name<<"'"<<endl;// write the name
	
	for (p=0;p<9;++p)
	{
		for (j=0;j<2;++j)
			File<< "<"<< p << ";" << j << ":" <<sounds[i].freq[p][j]<<">"<<endl;
	}
	for (p=0;p<17;++p)
		File<< "{"<< p << ":"<<sounds[i].choice[p]<<"}"<<endl;
	for (p=0;p<_PARACOUNT;++p)// write the remaining parameters
		File<< "("<< p << ":"<<sounds[i].parameter[p]<<")"<<endl;
 }// end of for i

File.close();

	std::string path = folder + "/minicomputerMemory.txt";
	if (access(path.c_str(), R_OK) == 0) // check if there a previous file which need to be backed up
	{
		std::string bak = path + ".bak";
		unlink(bak.c_str());
		rename(path.c_str(), bak.c_str());// make a backup
	}
	rename(temp.c_str(), path.c_str());// commit the file finally
}

/**
 * load the soundmemory from disk
 * supports the depricated binary and textformat.
 * @see Memory::save()
 */
void Memory::load()
{
std::string path = folder + "/minicomputerMemory.txt";
printf("loading %s",path.c_str());
ifstream File (path.c_str());
if (File.is_open())
	loadFromStream(File);
else
{
	std::string mem (
		#include "InitMemory.dat"
	);
	std::istringstream stream (mem);
	loadFromStream(stream);
}
}
void Memory::loadFromStream(std::istream &stream)
{
// *************************************************************
// new fileinput in textformat which is the way to go

string str,sParameter,sValue;
float fValue;
int iParameter,i2Parameter;
int current=-1;
unsigned int j;
getline(stream,str);
while (stream)
{
	sParameter="";
	sValue = "";
	switch (str[0])
	{
		case '(':// setting parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].parameter[iParameter]=fValue;
			}
		}
		break;
		case '{':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].choice[iParameter]=(int)fValue;
			}
		}
		break;
		case '<':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].freq[iParameter][i2Parameter]=fValue;
			}
		}
		break;
		case '\'': // setting the name
		{
			j = 1; // important, otherwise it would bail out at the first '	
			while ((j<str.length()) && (str[j]!='\'') && (j<128) )
			{
				sounds[current].name[j-1] = str[j];
				++j;
			}
			if (j<128) // fill the rest with blanks to clear the string
			{
				while (j<128)
				{
					sounds[current].name[j-1]=' ';
					++j;
				}
			}
		}
		break;
		case '[':// setting the current sound index
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				current = iParameter;
			}
		}
		break;

	}

	getline(stream,str);// get the next line of the file
}
}

/** export a single sound to a textfile
 * @param the filename
 * @param the sound memory location which is exported
 */
void Memory::exportSound(string filename,unsigned int current)
{
ofstream File (filename.c_str()); // temporary file
int p,j;
	//File<< "["<<i<<"]" <<endl;// write the soundnumber
	File<< "'"<<sounds[current].name<<"'"<<endl;// write the name
	
	for (p=0;p<9;++p)
	{
		for (j=0;j<2;++j)
			File<< "<"<< p << ";" << j << ":" <<sounds[current].freq[p][j]<<">"<<endl;
	}
	for (p=0;p<17;++p)
		File<< "{"<< p << ":"<<sounds[current].choice[p]<<"}"<<endl;
	for (p=0;p<_PARACOUNT;++p)// write the remaining parameters
		File<< "("<< p << ":"<<sounds[current].parameter[p]<<")"<<endl;

File.close();
}
/** import a single sound from a textfile
 * and write it to the given memory location
 * @param the filename
 * @param the sound memory location whose parameters are about to be overwritten
 */
void Memory::importSound(string filename,unsigned int current)
{
ifstream File (filename.c_str());
string str,sParameter,sValue;
float fValue;
int iParameter,i2Parameter;
unsigned int j;
getline(File,str);
while (File)
{
	sParameter="";
	sValue = "";
	switch (str[0])
	{
		case '(':// setting parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].parameter[iParameter]=fValue;
			}
		}
		break;
		case '{':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].choice[iParameter]=(int)fValue;
			}
		}
		break;
		case '<':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].freq[iParameter][i2Parameter]=fValue;
			}
		}
		break;
		case '\'': // setting the name
		{
			j = 1; // important, otherwise it would bail out at the first '	
			while ((j<str.length()) && (str[j]!='\'') && (j<128) )
			{
				sounds[current].name[j-1] = str[j];
				++j;
			}
			if (j<128) // fill the rest with blanks to clear the string
			{
				while (j<128)
				{
					sounds[current].name[j-1]=' ';
					++j;
				}
			}
		}
		break;
		/*case '[':// setting the current sound index
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				current = iParameter;
			}
		}
		break;*/

	}

	getline(File,str);// get the next line of the file
}
File.close();
// now the new sound is in RAM but need to be saved to the main file
save();
}
/**
 * the multitemperal setup, the choice of sounds and some settings
 * are saved to an extra file
 * supports the old depricated binary format and the new textformat like the sound saving
 * @see Memory::save()
 */
void Memory::saveMulti()
{
      int i;
//---------------------- new textformat
// first write in temporary file, just in case

std::string temp = folder + "/minicomputerMulti.temp";
ofstream File (temp.c_str()); // temporary file

int p,j;
for (i=0;i<128;++i)// write the whole 128 multis
{
	File<< "["<<i<<"]" <<endl;// write the multi id number
	File<< "'"<<multis[i].name<<"'"<<endl;// write the name of the multi
	
	for (p=0;p<8;++p) // store the sound ids of all 8 voices
	{
		File<< "("<< p << ":" <<multis[i].sound[p]<<")"<<endl;
	for (j=0;j<_MULTISETTINGS;++j)
		File<< "{"<< p << ";"<< j << ":" <<multis[i].settings[p][j]<<"}"<<endl;
	}
}

File.close();// done

	std::string path = folder + "/minicomputerMulti.txt";
	if (access(path.c_str(), R_OK) == 0) // check if there a previous file which need to be backed up
	{
		std::string bak = path + ".bak";
		unlink(bak.c_str());
		rename(path.c_str(), bak.c_str());// make a backup
	}
	rename(temp.c_str(), path.c_str());// commit the file
}
/**
 * load the multitemperal setups which are stored in an extrafile
 * and the new textformat
 * @see Memory::load
 * @see Memory::save
 */
void Memory::loadMulti()
{
std::string path = folder + "/minicomputerMulti.txt";
ifstream File (path.c_str());
if (File.is_open())
	loadMultiFromStream (File);
else
{
	std::string mem (
		#include "InitMulti.dat"
	);
	std::istringstream stream (mem);
	loadMultiFromStream(stream);
}
}
void Memory::loadMultiFromStream(std::istream &stream)
{
// *********************************** the new text format **********************
string str,sValue,sParameter;
int iParameter,i2Parameter;
float fValue;

getline(stream,str);// get the first line from the file
int current=0;
unsigned int j;
while (stream)// as long there is anything in the file
{
	// reset some variables
	sParameter="";
	sValue = "";
	// parse the entry (line) based on the first character
	switch (str[0])
	{
		case '(':// setting parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				multis[current].sound[iParameter]=(int)fValue;
			}
		}
		break;
		case '\'': // setting the name
		{
			j = 1; // important, otherwise it would bail out at the first '	
			while ((j<str.length()) && (str[j]!='\'') && (j<128) )
			{
				multis[current].name[j-1] = str[j];
				++j;
			}
			if (j<128) // fill the rest with blanks to clear the string
			{
				while (j<128)
				{
					multis[current].name[j-1]=' ';
					++j;
				}
			}
		}
		break;
		case '[':// setting the current sound index
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				current = iParameter;
			}
		}
		break;
		case '{':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				multis[current].settings[iParameter][i2Parameter]=fValue;
			}
		}
		break;

	}// end of switch

	getline(stream,str);// get the next line
}// end of while (file)
}

/**
 * parse parameter and values out of a given string
 * parameters are addresses of the actual variable to return the values with it
 *
 * @param string the line
 * @param int the first parameter
 * @param int the second, optional parameter
 * @param float the actual value
 * @return bool, true if the parsing worked
 */
bool Memory::parseNumbers(string &str,int &iParameter,int &i2Parameter,float &fValue)
{
 bool rueck = false;
 if (!str.empty())// just to make sure
 {
	istringstream sStream,fStream,s2Stream;
	string sParameter="";
	string sValue="";
	string sP2="";
	unsigned int index = 0;
	bool hasValue = false,hasP2 = false,isValue=false,isP2=false;
	// first getting each digit character
	while (index<str.length())// examine character per character
	{
		if ((str[index]>='0') && (str[index]<='9'))// is it a number?
		{
			if (isValue)
			{
				sValue+=str[index];
			}
			else if (isP2)
			{
				sP2+=str[index];
			}
			else
			{	
				sParameter+=str[index];
			}
		}
		else if (str[index] == '.')
		{
			if (isValue)
			{
				sValue+='.';
			}
		}
		else if (str[index] == '-')
		{
			if (isValue)
			{
				sValue+='-';
			}
		}
		else if (str[index] == ':')
		{
			hasValue = true;
			isValue = true;
			isP2 = false;
		}
		else if (str[index] == ';')
		{
			hasP2 	= true;
			isP2	= true;
			isValue = false;
		}
		++index;// next one please
	}
//	cout << sParameter<<" sp2 "<<sP2<<" value "<<sValue<<endl;
	// now actually turn them to ints or float
	sStream.str(sParameter);

	if (sStream>>iParameter)
	{
		rueck = true;
		if (hasValue)// turn the value into a float
		{
			fStream.str(sValue);
			if (fStream>>fValue)
				rueck = true;
			else
				rueck = false;
		}
		
		if (hasP2)// turn the second parameter into an int
		{
			s2Stream.str(sP2);
			if (s2Stream>>i2Parameter)
				rueck = true;
			else
				rueck = false;
		}
	}
 }
// cout << "p1: " << iParameter << " p2: " << i2Parameter<<" value: " << fValue<<" rueck: "<<rueck<<endl;
 return rueck;
}	

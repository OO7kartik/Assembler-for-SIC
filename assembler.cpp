#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <sstream>
#include <string>
#include "OPTAB.cpp"
#include "SYMTAB.cpp"
#include "line.cpp"

#define UNDEF -1

using namespace std;

class ASSEMBLER
{
	private:
	    string fname;
		OPTAB opTable;
		SYMTAB symTable;
		int size,start,LOCCTR;
		
		ifstream fin;
		ofstream fout;		
		
		string hexcode(string a)    //basically to convert strings to hexcode
		{
			string r;
			stringstream ss;
		
			for(auto c:a) ss<<hex<<int(c);   
			ss>>r;
			for(auto& x:r) x=toupper(x);  //to capitalize
			return r;
		}	
		string caps(string &s) 
		{
			for(auto& c:s) c=toupper(c);
			return s;
		}
		void MakeHeaderRecord();
		void MakeTextRecord();
		void MakeEndRecord(const string& lastLine)
		{
			line l(lastLine);
			stringstream ss,ss2;
			string s; 			
			ss<<std :: hex<<symTable.table[l.statement[OPERANDS]][0];
			ss>>s;
			fout<<"\nE^00"<<caps(s);			
			
			fout.seekp(16,ios::beg);						
			ss2<<hex<<LOCCTR-start;
			ss2>>s;
			fout<<"00"<<caps(s);
			
			codeLength();
		}
		
		void InitiateTextRecord()
		{
			stringstream ss;
			string s;			
			ss<<std :: hex<<LOCCTR - size;
			ss>>s;
			fout<<"\nT^00"<<caps(s)<<"^  ";
		}
		//void GetFirstInputLine();
		void ForwardReferencing(const string&);
		
		void codeLength()
		{
			int tlen;
			char c;
			ios::pos_type p;			
			string s;
			fstream file(fname.c_str(),ios::in|ios::out);
			if(file){					
				for(file.seekg(22,ios::beg);file;) {
					stringstream ss;
					tlen=0;
					while(file.get(c)) if(c==' ') break;
					p=file.tellg();												 
					while(file.get(c)) {if(c=='T') break; if(c!='^' && c!=' ') tlen++;}
					file.seekp(p-1); 
					ss<<setw(2)<<setfill('0')<<hex<<tlen/2;
					ss>>s;
					file<<caps(s);					
				}
				stringstream ss;					
				file.clear();
				file.seekp(p-1); 
				ss<<setw(2)<<setfill('0')<<hex<<tlen/2-4; //this is for last text record
				ss>>s;
				file<<caps(s);
				file.close();
			}
			else cout<<"ERROR";			
		}		

	public:		
		ASSEMBLER(const string& progName)
		{
			fname="obj_code_of_" + progName;
			fin.open(progName.c_str());
			fout.open(("obj_code_of_" + progName).c_str());
			size = 0;
			if(fin)
			{
				cout<<"File Opened Successfully\n";
				MakeHeaderRecord();
				MakeTextRecord();
			}
			else
				cout<<"File Open Error!\n";
		}

		~ASSEMBLER()
		{
			fin.close();
			fout.close();
		}
};

void ASSEMBLER :: MakeHeaderRecord()
{
	string buffer;
	getline(fin, buffer);
	line l(buffer);

	fout<<"H";
	if(l.statement[LABEL].size())
	{
		fout<<"^"<<l.statement[LABEL].substr(0, min(6, (int)l.statement[LABEL].size()));
		for(int i = 6 - min(6, (int)l.statement[LABEL].size()); i > 0; i--)
			fout<<" ";
	}
	else
		fout<<"^      ";
	if(l.statement[OPCODE] == "START")
	{
		start = stoi(l.statement[OPERANDS], 0, 16); 
		fout<<"^00"<<std :: hex<<start<<"^      ";                //l.statement[OPERANDS]; 
		LOCCTR = start;                                   //stoi(l.statement[OPERANDS], 0, 16);  
	}
	else
	{
		fout<<"^000000^      ";
		start = LOCCTR = 0x0000; 
	}
}

void ASSEMBLER :: MakeTextRecord()
{
	string buffer;
	bool isText = true; // To decide when to use new text record, true if we want new text record	
	getline(fin, buffer);
	line l(buffer);
	string objCode; 
	int noOfOC = 0, frflag = 0;
	
	while(l.statement[OPCODE].compare("END"))
	{
		objCode.clear();
		//Check if comment
		if(l.statement[LABEL].compare("."))
		{
			//Check if Label exits
			if(l.statement[LABEL].compare(" "))
			{
				map<string, vector<int> > :: iterator found = symTable.table.find(l.statement[LABEL]); 
				
				found = symTable.table.find(l.statement[LABEL]); 
				if(found != symTable.table.end())
				{
					if(found->second[0] == UNDEF)
					{
						symTable.table[l.statement[LABEL]][0] = LOCCTR;
						frflag = 1;
					}
					else 
					{
						cout<<"\nDuplicate label!!\n";
						exit(0);
					}
				}
				else
				{	
					symTable.table[l.statement[LABEL]].push_back(LOCCTR);
				}
			}

			//Search OPTAB for OPCODE
			if(opTable.table[l.statement[OPCODE]].compare("\0"))
			{
				string label; //This is label in OPERAND field, not in LABEL field
				objCode = opTable.table[l.statement[OPCODE]];
				noOfOC += 3;
				//Check for Addressing mode
				if(l.statement[OPERANDS][l.statement[OPERANDS].size() - 1] == 'X')
					label = l.statement[OPERANDS].substr(0, l.statement[OPERANDS].size() - 2);//For eg. LABEL STCH BUFFER,X
				else
					label = l.statement[OPERANDS];

				map<string, vector<int> > :: iterator searchLabel = symTable.table.find(label);

				if(searchLabel != symTable.table.end())
				{
					//If symbol value is not NULL
					if(searchLabel->second[0] != UNDEF)
					{
						stringstream ss;
						string temp;
						ss<<std :: hex<<symTable.table[label][0];
						ss>>temp;
						objCode += temp;
						if(l.statement[OPERANDS][l.statement[OPERANDS].size() - 1] == 'X')
						{
							stringstream ss;
							ss<<std :: hex<<(int)objCode[2] - 48 + 8; //Add 8 to make 'x' bit 1
							ss>>objCode[2];
						}
					}
					else
					{
						if(l.statement[OPERANDS].compare(" "))
						{
							symTable.table[label].push_back(LOCCTR + 1); //Forward Referencing
						}
						objCode += "0000";
					}
				}
				else
				{
					if(l.statement[OPERANDS].compare(" "))
					{
						symTable.table[label].push_back(UNDEF);
						symTable.table[label].push_back(LOCCTR + 1);
					}
					 //Forward Referencing
					//cout<<symTable.table[label][1];
					objCode += "0000";
				}

				LOCCTR += 3;
				noOfOC += 3;
				size = 3;
			}
			else if(!l.statement[OPCODE].compare("WORD"))
			{
				LOCCTR += 3;
				int operand = stoi(l.statement[OPERANDS]);
				stringstream ss;
				ss<<std :: hex<<operand;
				ss>>objCode;
				int len = objCode.size();
				//Padding with 0s to make it 3 bytes
				for(int i = 0; i < 6 - len; i++)
					objCode = "0" + objCode;
				noOfOC += 3;
				size = 3;
			}
			else if(!l.statement[OPCODE].compare("RESW"))
			{
				LOCCTR += 3 * stoi(l.statement[OPERANDS]);
				noOfOC += 3 * stoi(l.statement[OPERANDS]);
				size = 3 * stoi(l.statement[OPERANDS]);
			}
			else if(!l.statement[OPCODE].compare("RESB"))
			{
				LOCCTR += stoi(l.statement[OPERANDS]);
				noOfOC += stoi(l.statement[OPERANDS]);
				size = stoi(l.statement[OPERANDS]);
			}
			else if(!l.statement[OPCODE].compare("BYTE"))
			{
				//Format for byte is C'data_string' or X'data_string'
				if(l.statement[OPERANDS][0] == 'C')
				{	
					objCode += hexcode(l.statement[OPERANDS].substr(2, l.statement[OPERANDS].size() - 3));
					LOCCTR += l.statement[OPERANDS].size() - 3; 
					noOfOC += l.statement[OPERANDS].size() - 3;
					size = l.statement[OPERANDS].size() - 3;
				}
				else
				{
					objCode += l.statement[OPERANDS].substr(2, l.statement[OPERANDS].size() - 3);
					LOCCTR += (l.statement[OPERANDS].size() - 3) / 2; 
					noOfOC += (l.statement[OPERANDS].size() - 3) / 2;
					size = (l.statement[OPERANDS].size() - 3) / 2;
				}
			}
            caps(objCode);
			if(noOfOC <= 60)
			{
				if(frflag)
				{
					frflag = !frflag;
					ForwardReferencing(l.statement[LABEL]);
					isText = true;
					noOfOC = 0;
				}
				if(objCode.size())
				{
					if(isText)
					{
						InitiateTextRecord();
						isText = !isText;
					}
					fout<<"^"<<objCode;
				}
			}
			else
			{
				if(frflag)
				{
					frflag = !frflag;
					ForwardReferencing(l.statement[LABEL]);
					isText = true;
				}
				if(objCode.size())
				{
					InitiateTextRecord();
					fout<<"^"<<objCode;
					isText = false;
				}
				else
					isText = true;
				noOfOC = 0;
			}
		}
		getline(fin, buffer);
		l = buffer;	
	}	
	MakeEndRecord(buffer);
}

void ASSEMBLER :: ForwardReferencing(const string& label)
{
	stringstream ss;
	string s;
	for(int i = 1; i < symTable.table[label].size(); i++)
	{
		fout<<"\nT^00";	
		ss<<std :: hex<<symTable.table[label][i]<<"^02^"<<std :: hex<<symTable.table[label][0];
		ss>>s;			
		fout<<caps(s);
	}
	
	while(symTable.table[label].size() > 1)
		symTable.table[label].pop_back();	
}

int main()
{
	string fname;
	cout<<"Enter Filename: ";
	cin>>fname;
	ASSEMBLER sicAsm(fname);
	return 0;
}

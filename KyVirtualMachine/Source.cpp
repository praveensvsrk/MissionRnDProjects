#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <stack>
#include <iterator>
#include <iomanip>

using namespace std;

class KyObject {
	
public:
	int address;
	bool isSerialized = false;
	void setAddress(int address) {
		this->address = address;
	}
	virtual string getType() = 0;
	virtual string toString() = 0;
	virtual string serialize() = 0;
};

stack<KyObject*> stk;

class MemoryManager {
private:
	map<int, KyObject*> memory;
public:
	void putObject(KyObject* obj, int addr) {
		this->memory[addr] = obj;
	}
	KyObject* getObject(int addr) {
		return this->memory[addr];
	}
	void display() {
		cout << "Memory pool" << endl << endl;
		for (map<int, KyObject*>::const_iterator
			i = memory.begin(), end = memory.end(); i != end; i++) {
			cout << "Location: " << i->first << endl;
			cout << "Data structure: " << i->second->toString() << endl << endl;
		}
	}
}memMgr;


class KyInt : public KyObject {
private:
	int value;
public:
	KyInt(int value) {
		this->setValue(value);
	}
	string getType() {
		return NULL;
	}
	string toString() {
		string s;
		int temp = value;
		while (temp != 0) {
			s.push_back(temp % 10 + '0');
			temp /= 10;
		}
		reverse(s.begin(), s.end());
		return s;
	}
	void setValue(int value) {
		this->value = value;
	}
	string serialize() {
		string s;
		s.append("I");
		s.append(this->toString());
		s.append("\n");
		return s;
	}
};


class KyList : public KyObject {
private:
	vector<KyObject*> list;
public:
	string getType() {
		return "list";
	}
	string toString() {
		string s;
		s.reserve(50);
		s.append("[");
		int size = list.size();
		int i = 0;
		for (i; i < size-1; i++) {
			s.append(list[i]->toString());
			s.append(", ");
		}
		s.append(list[i]->toString());
		s.append("]");
		return s;
	}
	void append(KyObject* obj) {
		list.push_back(obj);
	}
	string serialize() {
		string s;
		if (isSerialized) {
			s.append("g" + to_string(this->address) + "\n");
			return s;
		}
		isSerialized = true;
		s.reserve(50);
		s.append("(lp" + to_string(this->address));
		s.append("\n");
		int size = list.size();
		for (int i=0; i < size; i++) {
			if (list.at(i)->isSerialized) {
				s.append("g" + to_string(list.at(i)->address) + "\n");
			}
			else 
				s.append(list.at(i)->serialize());
			s.append("a");
		}
		return s;
	}

};

class KyString : public KyObject {
private:
	string str;
public:
	KyString(string str) {
		this->setString(str);
	}
	string getType() {
		return "str";
	}
	string toString() {
		return "\"" + str + "\"";
	}
	void setString(string str) {
		this->str = str;
	}
	string serialize() {
		string s;
		if (isSerialized) {
			s.append("g" + to_string(this->address) + "\n");
			return s;
		}
		isSerialized = true;
		s.append("S'");
		s.append(str);
		s.append("'\n");
		s.append("p" + to_string(address) + "\n");
		return s;
	}
};

class KyDict : public KyObject {
private:
	map<KyObject*, KyObject*>m;
public:
	void addPair(KyObject *key, KyObject *value) {
		m[key] = value;
	}
	KyObject *getValue(KyObject *key) {
		return m[key];
	}
	string getType() {
		return "dict";
	}
	string toString() {
		string s;
		s.append("{ ");
		for (map<KyObject*, KyObject*>::const_iterator
			i = m.begin(), end = m.end(); i != end; i++) {
			s.append(i->first->toString() + " : ");
			s.append(i->second->toString() + ", ");
		}
		s[s.size() - 2] = ' ';
		s[s.size() - 1] = '}';
		return s;
	}
	string serialize() {
		string s;
		if (isSerialized) {
			s.append("g" + to_string(this->address) + "\n");
			return s;
		}
		isSerialized = true;
		int i = 0;
		s.reserve(100);
		s.append("(dp");
		s.append(to_string(this->address));
		s.append("\n");
		for (map<KyObject*, KyObject*>::const_iterator
			i = m.begin(), end = m.end(); i != end; i++) {
			s.append(i->first->serialize());
			if (i->second->isSerialized)
				s.append("g" + to_string(i->second->address));
			else
				s.append(i->second->serialize());
			s.append("s");
		}
		return s;
	}
};


int parseValue(string str, int *i) {
	int result = 0;
	while (str[*i] != '\n') {
		result *= 10;
		result += str[*i] - '0';
		*i += 1;
	}
	return result;
}

string parseString(string str, int *i) {
	string new_string;
	while (str[*i] != '\'') {
		new_string.push_back(str[*i]);
		*i += 1;
	}
	*i += 1;
	return new_string;
}

void deserialize_data() {

	ifstream inFile;
	inFile.open("input.kyser");
	stringstream buffer;
	buffer << inFile.rdbuf();
	string bufferString = buffer.str();
	inFile.close();

	int i = 0;
	char ch;
	int addr;

	KyDict *kyDict;
	KyInt *kyInt;
	KyList *kyList;
	KyString *kyStr;
	KyObject *item1, *item2, *item3;
	string str;

	while ((ch = bufferString[i++]) != '.') {
		switch (ch) {
		case '\n':
		case ' ':
			continue;
		case '(':
			ch = bufferString[i++];
			switch (ch) {
			case 'd':
				kyDict = new KyDict();
				item1 = kyDict;
				stk.push(item1);
				break;
			case 'l':
				kyList = new KyList();
				item1 = kyList;
				stk.push(item1);
				break;
			}
			break;
		case 'p':
			addr = parseValue(bufferString, &i);
			memMgr.putObject(stk.top(), addr);
			stk.top()->setAddress(addr);
			break;
		case 'S':
			i+=1;
			str = parseString(bufferString, &i);
			kyStr = new KyString(str);
			item1 = kyStr;
			stk.push(item1);
			stk.top()->setAddress(addr);
			break;
		case 'I':
			kyInt = new KyInt(parseValue(bufferString, &i));
			item1 = kyInt;
			stk.push(item1);
			break;
		case 'a':
			item1 = stk.top();
			stk.pop();
			kyList = static_cast<KyList*>(stk.top());
			kyList->append(item1);
			break;
		case 's':
			item1 = stk.top();
			stk.pop();
			item2 = stk.top();
			stk.pop();
			kyDict = static_cast<KyDict*>(stk.top());
			kyDict->addPair(item2, item1);
			break;
		case 'g':
			addr = parseValue(bufferString, &i);
			stk.push(memMgr.getObject(addr));
			break;
		}
	}
}


int main() {
	deserialize_data();
	memMgr.display();
	while (!stk.empty()) {
		cout << stk.top()->serialize();
		stk.pop();
	}
	cout << "." << endl;
	getchar();
	return 0;
}
/* ******************************************************************************************** */
	void breakLiteral(const string& literal, string& type, string& args, bool& none, bool& negative) {

		// Get the arguments
		size_t index = literal.find('(');
		if(index == string::npos) {
			type = literal;
		}
		else {
			type = literal.substr(0, index);
			args = literal.substr(index, string::npos);
		}

		// Get the "None" part
		index = type.find("None");
		if(index != string::npos) {
			type = type.substr(index + 4);
			none = true;
		}
		else none = false;

		// Get the negative part
		if(type.at(0) == '-') {
			type = type.substr(1);
			negative = true;
		}
		else negative = false;

		// Sanity check - can't have "None-Goal"
		if(none && negative) assert(false && "The both cases can not be true");
	}

	/* ******************************************************************************************** */
	void parseObjects(const std::string& _args, std::vector <std::string>& objects) {
		char args[256];
		sprintf(args, "%s", _args.c_str());
		char * pch;
		pch = strtok(args, ",()");
		while(pch != NULL) {
			string arg(pch);
			objects.push_back(arg);
			pch = strtok(pch+strlen(pch)+1, ",()");
		}
	}

	/* ******************************************************************************************** */
	void parseObjects(const string& _args, vector <size_t>& objects) {
		char args[256];
		sprintf(args, "%s", _args.c_str());
		char * pch;
		pch = strtok(args, ",()");
		while(pch != NULL) {
			string arg(pch);
			if(arg.at(0) == 'a') arg = arg.substr(3);
			objects.push_back(atoi(arg.c_str()));
			pch = strtok(pch+strlen(pch)+1, " ,.-");
		}
	}

	/* *********************************************************************************************************** *
	void sort(vector <Action*>& currActions, const vector <string>& actOrder) {
		vector <Action*> ordered;
		for(size_t order_idx = 0; order_idx < actOrder.size(); order_idx++) {
			for(size_t act_idx = 0; act_idx < currActions.size(); act_idx++) {
				if(actOrder[order_idx].compare(currActions[act_idx]->name_) == 0) {
					ordered.push_back(currActions[act_idx]);
					// break;		// There might be multiple actions which have the same name
				}
			}
		}
		assert((ordered.size() == currActions.size()) && "None should be lost!");
		currActions = ordered;
	}

	/* ******************************************************************************************** */
	bool Literals::equals(const Literals& literals) const {

		// Check the number of sets
		if(size() != literals.size()) return false;

		// Look for the same set in the other map
		map <string, set <string> >::const_iterator literalSetIt = this->begin();
		for(; literalSetIt != this->end(); literalSetIt++) {

			// Check existence of the second literal
			map <string, set <string> >::const_iterator literalSetIt2 = literals.find(literalSetIt->first);
			if(literalSetIt2 == literals.end()) return false;

			// Check the sizes
			if(literalSetIt->second.size() != literalSetIt2->second.size()) return false;

			// Check the contents of the two sets
			set <string>::const_iterator setIt = literalSetIt->second.begin();
			for(; setIt != literalSetIt->second.end(); setIt++) {
				if(literalSetIt2->second.count(*setIt) != 1) return false;
			}

		}

		return true;
	}

	/* ******************************************************************************************** */
	System::System(const System& sys) {

		equalities = sys.equalities;
		inequalities << sys.inequalities.str();
		bounds = sys.bounds;
		numEqualities = sys.numEqualities;
		numInEqualities = sys.numInEqualities;

		// Copy the nodes
		map <size_t, Node*>::const_iterator nodeIt = sys.nodes.begin();
		for(; nodeIt != sys.nodes.end(); nodeIt++)
			nodes[nodeIt->first] = new Node(NULL, vector <Node*>(), nodeIt->first);

		// Copy the children and the parents
		map <size_t, Node*>::const_iterator myNodeIt = nodes.begin();
		for(nodeIt = sys.nodes.begin(); nodeIt != sys.nodes.end(); nodeIt++, myNodeIt++) {
			for(size_t i = 0; i < nodeIt->second->children_.size(); i++) {
				Node* temp = nodeIt->second->children_[i];
				myNodeIt->second->children_.push_back(nodes[temp->id_]);
			}

			// The parent
			if(nodeIt->second->parent_ != NULL) 
				myNodeIt->second->parent_ = nodes[nodeIt->second->parent_->id_];
		}

		// Create the tree
		for(size_t i = 0; i < sys.trees.size(); i++) {
			trees.push_back(nodes[(size_t) sys.trees[i]->id_]);
		}
	}

	/* ******************************************************************************************** */
	Literals::Literals(bool preset) {

		if(preset) {
			set <string> emptySet;
			this->insert(make_pair("Goal", emptySet));
			this->insert(make_pair("Free", emptySet));
			this->insert(make_pair("PossibleJump", emptySet));
			this->insert(make_pair("Unsupported", emptySet));
			this->insert(make_pair("Diff", emptySet));
			this->insert(make_pair("InAir", emptySet));
			this->insert(make_pair("On", emptySet));
			this->insert(make_pair("OnGround", emptySet));
		}
	}

	/* ******************************************************************************************** */
	bool Literals::contains(const std::string& type, const std::string& args) const {
		Literals::const_iterator it = this->find(type);
		if(it == end()) return false;
		if(args.empty()) return true;
		return (it->second.find(args) != it->second.end());
	}

	/* ******************************************************************************************** */
	bool Literals::remove(const std::string& type, const std::string& args) {
		Literals::iterator literalSetIt = this->find(type);
		if(literalSetIt == end()) return false;
		if(args.empty()) {
			assert(1 == erase(type));
			return true;
		}
		std::set <std::string>::iterator setIt = literalSetIt->second.find(args);
		if(setIt == literalSetIt->second.end()) return false;
		assert(1 == literalSetIt->second.erase(args));
		return true;
	}

	/* ******************************************************************************************** */
	void Literals::add(const std::string& type, const std::string& args) {
		Literals::iterator literalSetIt = this->find(type);
		if(literalSetIt == end()) {
			std::set <std::string> newSet;
			if(!args.empty()) newSet.insert(args);
			insert(make_pair(type, newSet));
		}
		else if(!args.empty()) literalSetIt->second.insert(args);
	}

	/* ******************************************************************************************** */
	void replaceAll(string& str, const string& from, const string& to) {
		size_t start_pos = 0;
		char buf[16];
		while((start_pos = str.find(from, start_pos)) != string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}

	/* ******************************************************************************************** */
	void replaceAll(string& str, const string& from, size_t toValue) {
		size_t start_pos = 0;
		char buf[16];
		sprintf(buf, "%lu", toValue);
		string to(buf);
		while((start_pos = str.find(from, start_pos)) != string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}

	/* ******************************************************************************************** */
	Action* declareAction(const string& name, size_t numObjects, const string& arguments) {

		// Declare the arguments
		vector <string> preconditions, addEffects, deleteEffects, goalPres;

		// Parse the arguments to get the preconditions
		size_t preconditionsIndex = arguments.find('!', 0);
		istringstream preconditionsStr(arguments.substr(0, preconditionsIndex));
		for(string temp; getline(preconditionsStr, temp, '|');)
			preconditions.push_back(temp);

		// Parse the arguments to get the add effects
		size_t addEffectsIndex = arguments.find('!', preconditionsIndex + 1);
		istringstream addEffectsStr(arguments.substr(preconditionsIndex + 1, addEffectsIndex - preconditionsIndex - 1));
		for(string temp; getline(addEffectsStr, temp, '|');)
			addEffects.push_back(temp);

		// Parse the arguments to get the delete effects
		size_t deleteEffectsIndex = arguments.find('!', addEffectsIndex + 1);
		istringstream deleteEffectsStr(arguments.substr(addEffectsIndex + 1, deleteEffectsIndex - addEffectsIndex - 1));
		for(string temp; getline(deleteEffectsStr, temp, '|');)
			deleteEffects.push_back(temp);

		// Parse the arguments to get the delete effects
		size_t goalPresIndex = arguments.find('!', deleteEffectsIndex + 1);
		istringstream goalPresStr(arguments.substr(deleteEffectsIndex + 1, goalPresIndex - deleteEffectsIndex - 1));
		for(string temp; getline(goalPresStr, temp, '|');)
			goalPres.push_back(temp);

		// Create and return the action
		return new Action(name, numObjects, preconditions, addEffects, deleteEffects, goalPres, arguments);
	}

	/* ******************************************************************************************** */
	string Action::replace(const vector <size_t>& objs) {

		string def = def_;
		for(size_t obj_idx = 0; obj_idx < numObjects_; obj_idx++) {
			char str1[16], str2[16];
			sprintf(str1, "%lu", obj_idx);
			sprintf(str2, "%lu", objs[obj_idx]);
			planning::replaceAll(def, string("obj") + str1, objs[obj_idx]);
			planning::replaceAll(def, string("X") + str1, string("x") + str2);
			planning::replaceAll(def, string("Y") + str1, string("y") + str2);
			planning::replaceAll(def, string("W") + str1, string("w") + str2);
			planning::replaceAll(def, string("H") + str1, string("h") + str2);
		}

		return def;
	}

}


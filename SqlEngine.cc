/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  BTreeIndex btree;
  bool hasIndex;
  bool usesIndex = false;
  bool done = false;

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  IndexCursor init; 

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  //figure out if index exists
  if ((rc = btree.open(table + ".idx", 'r')) < 0) {
    hasIndex = false;
  }
  else{
    hasIndex = true;
  }
  
  int minValue = 0; //minValue is where you start searching
  int absValue = -1; //value is where you MUST search because of an existing equals condition. For example key = 10 and key > 5 means you only have to do one search at 10
  if(hasIndex)
  {
    //find out the minimum value needed
    for(unsigned i = 0; i < cond.size(); i++){
        switch(cond[i].attr){
          case 1: 
            switch (cond[i].comp) {
                case SelCond::EQ:
                    usesIndex = true;
                    absValue = atoi(cond[i].value);
                    break;
                case SelCond::GT:
                    usesIndex = true;
                    if(atoi(cond[i].value) > minValue){
                      minValue = atoi(cond[i].value);
                    }
                    break;
                case SelCond::LT: 
                    usesIndex = true;
                    break;
                case SelCond::GE:
                    if(atoi(cond[i].value) > minValue){
                      minValue = atoi(cond[i].value);
                    }
                    usesIndex = true;
                    break;
                case SelCond::LE:
                    usesIndex = true;
                    break;
            }
            break;
          case 2: //non-key attribute being checked, skip it
            continue;
        }
    }
  }
  //search with index
  if(usesIndex)
  {
    count = 0;
    if(absValue != -1)
      btree.locate(absValue, init);
    else
      btree.locate(minValue, init);

    while(btree.readForward(init,key,rid) == 0 && !done)
    {
        if ((rc = rf.read(rid, key, value)) < 0) {
          fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
          goto exit_select;
        }
        for (unsigned i = 0; i < cond.size(); i++){
          // compute the difference between the tuple value and the condition value
          switch (cond[i].attr) {
          case 1:
                diff = key - atoi(cond[i].value);
                //check critical key conditions (everything but Not Equals)
                switch (cond[i].comp){
                    case SelCond::EQ:
                        if (diff != 0) done = true;
                        break;
                    case SelCond::GT: //technically this should never happen
                        if (diff <= 0) done = true;
                        break;
                    case SelCond::LT: 
                        if (diff >= 0) done = true;
                        break;
                    case SelCond::GE: //technially this should never happen
                        if (diff < 0) done = true;
                        break;
                    case SelCond::LE:
                        if (diff > 0) done = true;
                        break;
                }
               break;
          case 2:
               diff = strcmp(value.c_str(), cond[i].value);
               break;
          }
          //check non-critical conditions and skip if not met
          switch (cond[i].comp) {
              case SelCond::EQ:
                  if (diff != 0) goto key_next_tuple;
                  break;
              case SelCond::NE:
                  if (diff == 0) goto key_next_tuple;
                  break;
              case SelCond::GT:
                  if (diff <= 0) goto key_next_tuple;
                  break;
              case SelCond::LT:
                  if (diff >= 0) goto key_next_tuple;
                  break;
              case SelCond::GE:
                  if (diff < 0) goto key_next_tuple;
                  break;
              case SelCond::LE:
                  if (diff > 0) goto key_next_tuple;
                  break;
          }
        }
        //conditions have been met
        count++;
        //print tuple
        switch (attr) {
            case 1:  // SELECT key
              fprintf(stdout, "%d\n", key);
              break;
            case 2:  // SELECT value
              fprintf(stdout, "%s\n", value.c_str());
              break;
            case 3:  // SELECT *
              fprintf(stdout, "%d '%s'\n", key, value.c_str());
              break;
        }

        key_next_tuple:  //readforward takes care of incrementing
        true; //C++ won't let me put nothing after a goto label
    }
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }
    rc = 0;

  // close the table file and return
    im_fkin_done:
    rf.close();
    return rc;
  }
  else{
  rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }

    // check the conditions on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
      // compute the difference between the tuple value and the condition value
      switch (cond[i].attr) {
      case 1:
	         diff = key - atoi(cond[i].value);
	         break;
      case 2:
	         diff = strcmp(value.c_str(), cond[i].value);
	         break;
      }

      // skip the tuple if any condition is not met
      switch (cond[i].comp) {
          case SelCond::EQ:
            	if (diff != 0) goto next_tuple;
            	break;
          case SelCond::NE:
            	if (diff == 0) goto next_tuple;
            	break;
          case SelCond::GT:
            	if (diff <= 0) goto next_tuple;
            	break;
          case SelCond::LT:
            	if (diff >= 0) goto next_tuple;
            	break;
          case SelCond::GE:
            	if (diff < 0) goto next_tuple;
            	break;
          case SelCond::LE:
            	if (diff > 0) goto next_tuple;
            	break;
      }
    }

    // the condition is met for the tuple. 
    // increase matching tuple counter
    count++;

    // print the tuple 
    switch (attr) {
        case 1:  // SELECT key
          fprintf(stdout, "%d\n", key);
          break;
        case 2:  // SELECT value
          fprintf(stdout, "%s\n", value.c_str());
          break;
        case 3:  // SELECT *
          fprintf(stdout, "%d '%s'\n", key, value.c_str());
          break;
    }

    // move to the next tuple
    next_tuple:
    ++rid;
  }

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  fstream myFile;
  myFile.open(loadfile.c_str());

  RecordFile rec;
  rec.open(table + ".tbl", 'w');

  BTreeIndex* btree;
  if(index == true){
    btree = new BTreeIndex();
    btree->open(table + ".idx", 'w');
  }

  int key;
  string value;

  char line[256];
  while(!((myFile.getline(line, 256)).fail())){
      const string s = line;
      if(parseLoadLine(s, key, value) == 0){
          RecordId meh = rec.endRid();
          if(index == true){
            btree->insert(key, meh);
          }
          rec.append((int)key, value, meh);
      }
  }
  if(index == true)
    btree->close();

  return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}

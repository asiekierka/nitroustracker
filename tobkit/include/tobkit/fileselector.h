/*====================================================================
Copyright 2006 Tobias Weyand

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
======================================================================*/

#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include "listbox.h"
#include <nds.h>

#include <vector>
#include <map>
#include <string>

namespace tobkit {

typedef std::map<std::string, std::vector<std::string> > FilterSet;

class File {
	public:
		std::string name;
		std::string name_with_path;
		u8 order;
		bool is_dir;
		// u32 size;
};

class FileSelector: public ListBox {
	public:
		FileSelector(u8 _x, u8 _y, u8 _width, u8 _height, Screen *_screen, bool visible=true);
		
		// Calls fileselect callback or changes the directory
		void penDown(u8 px, u8 py);
	
		// Drawing request
		void pleaseDraw(void);

		// File list invalidation
		void invalidateFileList(void);

		// Sets the file select callback
		void registerFileSelectCallback(void (*onFileSelect_)(File));
		
		// Sets the dir change callback
		void registerDirChangeCallback(void (*onDirChange_)(const char *newdir));
		
		// Defines a filter rule, selects it if it's the first rule, updates view
		void addFilter(std::string filtername, std::vector<std::string> extensions);
	
		// Selects a filter rule and upates view
		void selectFilter(std::string filtername);
	
		// Get pointer to the selcted file, 0 is no file selected
		File *getSelectedFile(void);
	
		// Get current dir
		std::string getDir(void);
		
		// Set current dir
		void setDir(std::string dir);
	
	protected:
		void draw(void);

	private:
		// Helper for converting a string to lowercase
		//void lowercase(char *str);
	
		// Reads the current directory
		// stores the alpabetically sorted list of files/dis in filelist and updates view
		// Does not handle anything not in the filter
		// Handles everything if there's no filter
		void read_directory(void);
		
		std::string current_directory;
		FilterSet filters;
		std::string active_filterset;
		void (*onFileSelect)(File);
		void (*onDirChange)(const char *newdir);
		
		std::vector<File> filelist; // Files that are displayed
		bool filelist_refresh; // Request refresh for file list

		// Hack! ListBox::penDown calls draw(), which we capture and perform later.
		bool ignore_draws;
		bool parent_requested_draw;
};

};

#endif

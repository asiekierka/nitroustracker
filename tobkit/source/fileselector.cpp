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

#include <sys/syslimits.h>
#include <unistd.h>

#include <fat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>

#include "tobkit/fileselector.h"

/* ===================== PUBLIC ===================== */

FileSelector::FileSelector(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, bool visible)
	:ListBox(_x, _y, _width, _height, _vram, 0, false, visible),
	current_directory("/"), active_filterset(""), filelist_refresh(true),
	ignore_draws(false), parent_requested_draw(false)
{
	onFileSelect = 0;
	onDirChange  = 0;
}

// Drawing request
void FileSelector::pleaseDraw(void) {
	if(filelist_refresh) {
		read_directory();
		filelist_refresh = false;
	}
	ListBox::pleaseDraw();
}

// File list invalidation request
void FileSelector::invalidateFileList(void) {
	filelist_refresh = true;
}

// Calls fileselect callback or changes the directory
void FileSelector::penDown(u8 px, u8 py)
{
	bool touched_entry = true;

	ignore_draws = true;
	ListBox::penDown(px, py);
	ignore_draws = false;

	// Don't do anything if the scrollthingy is touched!
	u8 relx = px-x;
	if(relx>=width-SCROLLBAR_WIDTH) {
		touched_entry = false;
	}

	// Quit if an out of range element was tapped
	if(activeelement>=elements.size()) touched_entry = false;

	if(touched_entry) {
		// If it is a dir, enter it
		if((filelist.at(activeelement).is_dir == true)&&(filelist.at(activeelement).name != "..")) {
			//iprintf("element %u\n",activeelement);
			current_directory += filelist.at(activeelement).name + "/";
			//iprintf("newdir: %s\n",current_directory.c_str());
			activeelement = 0;

			if(onDirChange != NULL) {
				onDirChange(current_directory.c_str());
			}

			invalidateFileList();

		// If it is "..", go down a directory
		} else if(filelist.at(activeelement).name == "..") {
			std::string name = current_directory;
			auto slashpos = name.find_last_of("/", name.length()-2);
			if(slashpos != std::string::npos) {
				name.erase(slashpos, name.length()-slashpos-1);
				current_directory = name;
				//iprintf("%s\n",current_directory.c_str());
				activeelement = 0;

				if(onDirChange != NULL) {
					onDirChange(current_directory.c_str());
				}

				invalidateFileList();
			}
		// If it is a file, call the callback
		} else if(onFileSelect != 0) {
			onFileSelect(filelist.at(activeelement));
		}
	}

	if (parent_requested_draw || filelist_refresh) {
		pleaseDraw();
		parent_requested_draw = false;
	}
}

// Sets the file select callback
void FileSelector::registerFileSelectCallback(void (*onFileSelect_)(File)) {
	onFileSelect = onFileSelect_;
}

// Sets the dir change callback
void FileSelector::registerDirChangeCallback(void (*onDirChange_)(const char *newdir)) {
	onDirChange = onDirChange_;
}

// Defines a filter rule, selects it if it's the first rule, updates view
void FileSelector::addFilter(std::string filtername, std::vector<std::string> extensions)
{
	filters[filtername] = extensions;
	if(filters.size()==1) {
		active_filterset = filtername;
	}

	invalidateFileList();
}

// Selects a filter rule and upates view
void FileSelector::selectFilter(std::string filtername)
{
	active_filterset = filtername;

	invalidateFileList();
}

// Get pointer to the selcted file, 0 is no file selected
File *FileSelector::getSelectedFile(void)
{
	if(activeelement < filelist.size()) {
		if(filelist.at(activeelement).name != "..") {
			return &(filelist.at(activeelement));
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

// Get current dir
std::string FileSelector::getDir(void) {
	return current_directory;
}

// Set current dir
void FileSelector::setDir(std::string dir)
{
	if (!dir.ends_with("/")) dir += "/";
	current_directory = dir;
}

/* ===================== PROTECTED ===================== */

void FileSelector::draw(void)
{
	if(!ignore_draws) {
		ListBox::draw();
	} else {
		parent_requested_draw = true;
	}
}

/* ===================== PRIVATE ===================== */

std::string stringtolowercase(std::string str)
{
	std::string outstr = str;
	for(u8 i=0;i<str.size();++i) {
		outstr[i] = tolower(outstr[i]);
	}
	return outstr;
}

inline bool compare_filenames(File f1, File f2)
{
	if(f1.order != f2.order) {
		return f1.order < f2.order;
	} else if(strcasecmp(f1.name.c_str(),f2.name.c_str())<0) {
		return true;
	} else {
		return false;
	}
}

// Reads the current directory
// stores the alpabetically sorted list of files/dirs in filelist and updates view
// Does not handle anything not in the filter
// Handles everything if there's no filter
void FileSelector::read_directory(void)
{
	// Go though dir and collect files
	filelist.clear();

	if( chdir(current_directory.c_str()) == -1 ) {
		iprintf("cwd to %s failed\n", current_directory.c_str());
		return;
	}

	DIR *dir;
	struct stat filestats;

	if((dir = opendir(current_directory.c_str())) == NULL)
	{
		iprintf("Dir read error!\n");
		return;
	}

	struct dirent *direntry = readdir(dir);

	char filename[PATH_MAX];
	while(direntry != NULL)
	{
		if(direntry->d_name[0] != '.') { // Hidden and boring files

			File newfile;
			newfile.name = direntry->d_name;
			newfile.name_with_path = current_directory + direntry->d_name;
			newfile.is_dir = (direntry->d_type == DT_DIR);
			newfile.order = newfile.is_dir ? 1 : 2;

            if(!newfile.is_dir) {
                int stat_res = stat(newfile.name_with_path.c_str(), &filestats);
                if(stat_res != -1) {
        			newfile.size = filestats.st_size;
                }
            }
			filelist.push_back(newfile);
		}
		direntry = readdir(dir);
	}

	closedir(dir);
	// Apply filter if there is one
	if(active_filterset != "") {
		std::vector<File> newfilelist;
		std::vector<File>::const_iterator fileit;
		std::string extension;
		for(fileit=filelist.begin();fileit!=filelist.end();++fileit) {
			if(fileit->is_dir == true)
			{
				// Don't filter dirs
				newfilelist.push_back(*fileit);
			} else {
				u32 extensionsize;
				auto lastdot = fileit->name.find_last_of(".");
				if(lastdot != std::string::npos) {
					extensionsize = fileit->name.size() - fileit->name.find_last_of(".") - 1;
				} else {
					extensionsize = 0;
				}
				extension = stringtolowercase(fileit->name.substr(fileit->name.size()-extensionsize,extensionsize));
				if(find(filters[active_filterset].begin(), filters[active_filterset].end(),extension) != filters[active_filterset].end()) {
					newfilelist.push_back(*fileit);
				}
			}
		}

		filelist = newfilelist;
	}

	// Add ".."
	if(current_directory != "/" && !current_directory.ends_with(":/")) {
		File dotdot;
		dotdot.name = "..";
		dotdot.is_dir = true;
		dotdot.order = 0;
		dotdot.size = 0;
		filelist.push_back(dotdot);
	}

	// Sort
	sort(filelist.begin(), filelist.end(), compare_filenames);
	// Display
	elements.clear();
	activeelement = 0;
	scrollpos = 0;

	std::string newentry;
	for(u16 i=0;i<filelist.size();++i) {
		newentry = filelist.at(i).name;
		if(filelist.at(i).is_dir == true) {
			newentry = "[" + newentry + "]";
		}
		strncpy(filename, newentry.c_str(), 255);
		elements.push_back(filename);
	}
}

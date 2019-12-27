#include <dirent.h>
#include <sys/stat.h>
#include <bits/stdc++.h>

using namespace std;
vector<pair<string,string> > v;

void explore(char* dirName){
	DIR* directory;
	struct dirent *entry;
	struct stat info;
	
	directory = opendir(dirName);
	if(!directory){
		return;
	}
	while((entry = readdir(directory)) != NULL){
		if(entry->d_name[0] != '.'){
			string path = string(dirName) + "/" + string(entry->d_name);
                        //printf("Searching... %s\n", path.c_str());
			stat(path.c_str(), &info);
			if(info.st_mode & S_IFDIR )
				v.push_back(make_pair(path,"directory"));
			else if(info.st_mode & S_IFREG)
				v.push_back(make_pair(path,"file"));
			else
				v.push_back(make_pair(path,"link")); 
			if(S_ISDIR(info.st_mode)){
				explore((char*)path.c_str());
			}
		}
	}
	closedir(directory);
}

int main(int argc, char** argv){
	int flag = 0;
	explore((char*)".");
	string arg(argv[1]);
        //printf("Search Results\n");
        int occur = 0;
	for(int i = 0; i < v.size(); i++){
		if(v[i].first.find(arg) != -1){
			if(v[i].first.substr(v[i].first.length()-arg.length(),arg.length()).compare(arg) == 0 && v[i].first[v[i].first.length()-arg.length()-1] == '/'){
				cout<<v[i].first<<"   ---   "<<v[i].second<<endl;
				flag = 1;
                                occur++;
			}
		}
	}
        if(flag)
            cout<<"Number of occurence : "<<occur<<endl;
	if(!flag)
	    cout<<"Does not exist\n";
}

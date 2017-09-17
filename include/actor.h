/* Copyright 2017 Cristian Creteanu & George Cristian Munteanu */
#include <map>
#include <string>

struct actor {
	std::string id;
	std::string name;  // numele actorului
	std::string firstMovie;  // primul film in care a jucat
	std::string lastMovie;  // ultimul film in care a jucat
	std::map<std::string, int> colleagues;  // colegii de gradul 1
	int activitySpan = -1;  // durata carierei lui
};

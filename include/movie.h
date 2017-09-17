/* Copyright 2017 Cristian Creteanu & George Cristian Munteanu */
#include <string>
#include <vector>
#include <unordered_map>

struct movie {
	std::string name;  // numele filmului
	std::string id;  // id-ul filmului
	int timestamp;
	std::vector<std::string> categories;  // categoriile din care face parte
	std::string director_name;  // numele regizorului
	std::vector<std::string> actors_ids;
	// ratings este va retine perechile (user, rating)
	std::unordered_map<std::string, int> ratings;
	// sumRatings retine suma rating-urilor, iar noRatings numarul lor
	// pentru a calcula mai eficient average rating
	int sumRatings = 0, noRatings = 0;
};

struct moviePlusTimestamp {
    std::string movie_id;
    int timestamp;
};

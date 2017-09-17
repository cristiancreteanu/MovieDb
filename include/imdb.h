/* Copyright 2017 Cristian Creteanu & George Cristian Munteanu */
#ifndef __IMDB__H__
#define __IMDB__H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>

#define NONE                            "none"

#define ADD_MOVIE                       "add_movie"
#define ADD_USER                        "add_user"
#define ADD_ACTOR                       "add_actor"
#define ADD_RATING                      "add_rating"
#define UPDATE_RATING                   "update_rating"
#define REMOVE_RATING                   "remove_rating"
// queries
#define GET_RATING                      "get_rating"
#define GET_LONGEST_CAREER_ACTOR        "get_longest_career_actor"
#define GET_MOST_INFLUENTIAL_DIRECTOR   "get_most_influential_director"
#define GET_BEST_YEAR_FOR_CATEGORY      "get_best_year_for_category"
#define GET_2ND_DEGREE_COLLEAGUES       "get_2nd_degree_colleagues"
#define GET_TOP_K_MOST_RECENT_MOVIES    "get_top_k_most_recent_movies"
#define GET_TOP_K_ACTOR_PAIRS           "get_top_k_actor_pairs"
#define GET_TOP_K_PARTNERS_FOR_ACTOR    "get_top_k_partners_for_actor"
#define GET_TOP_K_MOST_POPULAR_MOVIES   "get_top_k_most_popular_movies"
#define GET_AVG_RATING_IN_RANGE         "get_avg_rating_in_range"

#include "./movie.h"
#include "./actor.h"
#include "./user.h"
#include "./director.h"
#include "./actorPair.h"

class IMDb {
 public:
	IMDb();
	~IMDb();

	void add_movie(std::string movie_name, std::string movie_id, int timestamp,
				   std::vector<std::string> categories,
				   std::string director_name,
				   std::vector<std::string> actor_ids);

	void add_user(std::string user_id, std::string name);
	void add_actor(std::string actor_id, std::string name);

	void add_rating(std::string user_id, std::string movie_id, int rating);
	void update_rating(std::string user_id, std::string movie_id, int rating);
	void remove_rating(std::string user_id, std::string movie_id);

	// queries
	std::string get_rating(std::string movie_id);
	std::string get_longest_career_actor();
	std::string get_most_influential_director();
	std::string get_best_year_for_category(std::string category);
	std::string get_2nd_degree_colleagues(std::string actor_id);
	std::string get_top_k_most_recent_movies(int k);
	std::string get_top_k_actor_pairs(int k);
	std::string get_top_k_partners_for_actor(int k, std::string actor_id);
	std::string get_top_k_most_popular_movies(int k);
	std::string get_avg_rating_in_range(int start, int end);

	// input reader helper.
	std::string read_input_line(const std::string& line);

 private:
	// unordered map-ul movies va contine filmele cu toate informatiile necesare
	std::unordered_map<std::string, movie> movies;

	// unordered map-ul mostPopularMovies va contine filmele ordanate dupa
	// numarul de rating-uri si id
	std::map<movie, std::string> mostPopularMovies;

	// variabila moviesUpdated va fi true dupa ce se actualizeaza movies
	// va deveni false dupa ce se va actualiza mostPopularMovies
	bool moviesUpdated = true;

	// unordered map-ul actors va contine actorii cu toate informatiile necesare
	std::unordered_map<std::string, actor> actors;

	// unoredered map-ul users va retine utilizatorii
	std::unordered_map<std::string, user> users;

	// genres va retine categoriile de filme si pentru fiecare dintre acestea
	// filmele care apartin categoriei
	std::unordered_map<std::string, std::set<moviePlusTimestamp> > genres;

	// pairs va retine perechile ((actor1, actor2), numar de filme)
	std::map<std::pair <std::string, std::string>, int> pairs;

	// pairsTop va contine perechile mentionate anterior ordonate
	// dupa numarul de filme
	std::map<actorPair, int> pairsTop;

	// variabila pairsUpdated va fi true dupa ce se actualizeaza pairs
	// va deveni false dupa ce se va actualiza pairsTop
	bool pairsUpdated = true;

	// directors va contine regizorii cu toate informatiile necesare
	std::unordered_map<std::string, director> directors;

	std::set<moviePlusTimestamp> topRecentMovies;

	// map cu perechi (movie timestamp, movie id)
	std::map<int, std::string> timestamps;
};

#endif

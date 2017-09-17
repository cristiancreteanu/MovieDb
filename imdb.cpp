/* Copyright 2017 Cristian Creteanu & George Cristian Munteanu */
#include <math.h>
#include <time.h>
#include <iterator>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <iostream>
#include <map>
#include <utility>

#include "include/imdb.h"

// comparare regizori dupa numarul de actori cu care au colaborat si dupa nume
bool operator <(const director& d1, const director& d2) {
	return d1.actors.size() != d2.actors.size() ?
		d1.actors.size() > d2.actors.size() : d1.name < d2.name;
}

// comparare perechi de actori dupa numarul de filme in care au jucat si dupa
// numele lor
bool operator <(const actorPair& p1, const actorPair& p2) {
	return p1.noMovies != p2.noMovies ? p1.noMovies > p2.noMovies :
			p1.act1 != p2.act1 ? p1.act1 < p2.act1 :
			p1.act2 < p2.act2;
}

// comparare filme dupa numarul de rating-uri si id
bool operator <(const movie& m1, const movie& m2) {
	return m1.ratings.size() != m2.ratings.size() ?
			m1.ratings.size() > m2.ratings.size() : m1.id < m2.id;
}

// comparare filme dupa timestamp si id
bool operator <(const moviePlusTimestamp& m1, const moviePlusTimestamp& m2) {
	return m1.timestamp != m2.timestamp? m1.timestamp < m2.timestamp :
			m1.movie_id < m2.movie_id;
}

// functia primeste un map cu perechi (id, movie) si intoarce un map cu
// perechi (movie, id) ordonat dupa noua cheie, movie
std::map<movie, std::string> flip_movie(
							std::unordered_map<std::string, movie> src) {
	std::map<movie, std::string> dst;

	for (auto it = src.begin(); it != src.end(); it++) {
		dst.insert(std::pair<movie, std::string>(it->second, it->first));
	}

	return dst;
}

// functia primeste un map cu perechi ((actor1, actor2), numar de filme) si
// intoarce un map cu perechi (numar de filme, (actor1, actor2)) ordonat dupa
// noua cheie, movie
std::map<actorPair, int> flip_pairs(const std::map<std::pair<std::string,
													std::string>, int > &src) {
	std::map<actorPair, int> dst;
	actorPair p;

	for (auto it = src.begin(); it != src.end(); it++) {
		p.act1 = it->first.first;
		p.act2 = it->first.second;
		p.noMovies = it->second;
		dst.insert(std::pair<actorPair, int>(p, it->second));
	}

	return dst;
}

IMDb::IMDb() {}

IMDb::~IMDb() {}

void IMDb::add_movie(std::string movie_name,
					 std::string movie_id,
					 int timestamp,  // unix timestamp when movie was launched
					 std::vector<std::string> categories,
					 std::string director_name,
					 std::vector<std::string> actor_ids) {
	int newDirector = 0;
	std::string firstMovie, lastMovie, firstDegree1, firstDegree2;

	moviesUpdated = true;

	movie film;
	film.name = movie_name;
	film.id = movie_id;
	film.timestamp = timestamp;
	film.categories = categories;
	film.director_name = director_name;
	film.actors_ids = actor_ids;
	movies.insert(std::pair<std::string, movie>(movie_id, film));

	// introduc o noua intrare in map-ul de filme cu timestamp
	timestamps.insert(std::pair<int, std::string>(timestamp, movie_id));

	moviePlusTimestamp m;
	m.movie_id = movie_id;
	m.timestamp = timestamp;
	topRecentMovies.insert(m);  // introduc un element nou in set-ul de filme

	if (directors.find(director_name) == directors.end()) {
		newDirector = 1;
		director d;
		d.name = director_name;
		d.actors = actor_ids;
		directors.insert(std::pair<std::string, director >(director_name, d));
	}

	for (auto it = actor_ids.begin(); it != actor_ids.end(); it++) {
		// de adauga in directors actorii
		if (newDirector == 0) {
			if (find(directors[director_name].actors.begin(),
					directors[director_name].actors.end(), *it)
					== directors[director_name].actors.end()) {
				directors[director_name].actors.push_back(*it);
				newDirector = 1;
			}
		}

		firstMovie = actors[*it].firstMovie;
		lastMovie = actors[*it].lastMovie;
		// cica actorul se afla in actors, asa ca nu mai tre' sa l caut
		if (firstMovie.empty()) {  // daca firstMovie e gol
			actors[*it].firstMovie = movie_id;  // se baga movie_id si in first
		} else if (lastMovie.empty()){  // daca doar lastMovie e gol
			// daca e mai recent movie_id
			if (movies[firstMovie].timestamp > movies[movie_id].timestamp) {
				// baga in lastMovie firstMovie
				actors[*it].lastMovie = actors[*it].firstMovie;
				actors[*it].firstMovie = movie_id;  // baga in firstMovie movie_id
			} else {  // daca nu e mai recent decat primul
				actors[*it].lastMovie = movie_id;  // se baga in lastMovie
			}
		} else {  // daca niciunul nu e gol
			// daca e mai recent movie_id decat primul
			if (movies[movie_id].timestamp < movies[firstMovie].timestamp) {
				actors[*it].firstMovie = movie_id;
			} else {
				// daca e mai recent decat al doilea
				if (movies[movie_id].timestamp > movies[lastMovie].timestamp) {
					actors[*it].lastMovie = movie_id;
				}
			}
		}

		if (actors[*it].lastMovie.empty()) {
			actors[*it].activitySpan = 0;
		} else {
			actors[*it].activitySpan = movies[actors[*it].lastMovie].timestamp -
										movies[actors[*it].firstMovie].timestamp;
		}

		for (auto it2 = actor_ids.begin(); it2 != actor_ids.end(); it2++) {
			if (it == it2)
				continue;

			if (actors[*it].colleagues.find(*it2) == actors[*it].colleagues.end()) {
				actors[*it].colleagues[*it2] = 1;
			} else {
				actors[*it].colleagues[*it2]++;
			}

			if (*it < *it2) {
				if (pairs.find(make_pair(*it, *it2)) != pairs.end()) {
					pairs[make_pair(*it, *it2)]++;
					pairsUpdated = true;
				} else {
					pairs.emplace(make_pair(*it, *it2), 1);
					pairsUpdated = true;
				}
			}
		}
	}

	// trec prin toate categoriile
	for (auto it = categories.begin(); it != categories.end(); it++) {
		if (genres.find(*it) != genres.end()) {  // daca categoria a fost introdusa
			moviePlusTimestamp m;
			m.movie_id = movie_id;
			m.timestamp = movies[movie_id].timestamp;
			genres[*it].insert(m);  // bag filmul la key-ul ei
		} else {  // daca nu a fost introdusa
			// fac un vector in care pun doar filmul curent
			std::set<moviePlusTimestamp> films;
			moviePlusTimestamp m;
			m.movie_id = movie_id;
			m.timestamp = movies[movie_id].timestamp;
			films.insert(m);
			// introduc o noua intrare in genres
			genres.insert(std::pair<std::string, std::set<moviePlusTimestamp> >(*it,
																				films));
		}
	}
}

void IMDb::add_user(std::string user_id, std::string name) {
	user u;
	u.name = name;
	u.id = user_id;

	users.insert(std::pair<std::string, user>(user_id, u));
}

void IMDb::add_actor(std::string actor_id, std::string name) {
	actor a;
	a.name = name;
	a.id = actor_id;

	actors.insert(std::pair<std::string, actor>(actor_id, a));
}

void IMDb::add_rating(std::string user_id, std::string movie_id, int rating) {
	movies[movie_id].ratings.insert(std::pair<std::string, int>(user_id, rating));
	// adun ratingul si maresc si numarul raitingurilor date la acest film
	movies[movie_id].sumRatings += rating;
	movies[movie_id].noRatings++;
	moviesUpdated = true;
}

void IMDb::update_rating(std::string user_id, std::string movie_id,
														int rating) {
	// scad ratingul dat anterior si il aduna pe cel nou
	movies[movie_id].sumRatings -= movies[movie_id].ratings[user_id];
	movies[movie_id].sumRatings += rating;
	movies[movie_id].ratings[user_id] = rating;
	moviesUpdated = true;
}

void IMDb::remove_rating(std::string user_id, std::string movie_id) {
	movies[movie_id].sumRatings -= movies[movie_id].ratings[user_id];
	movies[movie_id].noRatings--;
	movies[movie_id].ratings.erase(user_id);
	moviesUpdated = true;
}

std::string IMDb::get_rating(std::string movie_id) {
	if (!movies[movie_id].ratings.empty()) {  // daca filmul nu are rating
		double finalRating = 0;

		// calculez rating mediu impartind suma de rating-uri la numarul de rating-uri
		finalRating = 1.0 * movies[movie_id].sumRatings / movies[movie_id].noRatings;
		if ((int)finalRating == 10) {
			// rotunjesc la a doua zecimala
			return std::to_string(1.0 * floor(100 * finalRating + 0.5) / 100).substr(0,
																					5);
		}

		return std::to_string(1.0 * floor(100 * finalRating + 0.5) / 100).substr(0,
																				4);
	}

	return "none";
}

std::string IMDb::get_longest_career_actor() {
	int noMovies = 0;
	std::string longestActivitySpanActor;
	int maxActivitySpan = 0;  // initializare maxim

	// parcurg map-ul de actori
	for (auto it = actors.begin(); it != actors.end(); it++) {
		if (it->second.firstMovie.empty())  // daca n-a jucat in niciun film
			continue;

		if (it->second.lastMovie.empty()) {  // daca a jucat doar intr-un film
			if (longestActivitySpanActor.empty()) {
				longestActivitySpanActor = it->first;
				noMovies++;
			} else if (maxActivitySpan == 0 && longestActivitySpanActor > it->first) {
				longestActivitySpanActor = it->first;
			}
		}

		noMovies++;

		if (maxActivitySpan < it->second.activitySpan) {
			longestActivitySpanActor = it->first;
			maxActivitySpan = it->second.activitySpan;
		} else if (maxActivitySpan == it->second.activitySpan) {
			if (longestActivitySpanActor > it->first) {
				longestActivitySpanActor = it->first;
			}
		}
	}

	if (noMovies == 0)
		return "none";

	return longestActivitySpanActor;
}

std::string IMDb::get_most_influential_director() {
	if (directors.empty())
		return "none";

	unsigned int maxActors = 0;
	std::string mInfluentionDirector;

	// aflu intrarea din directors care are numarul de actori maxim
	// in caz de egalitate, iau directorul cu numele mai mic dpdv lexico-grafic
	for (auto it = directors.begin(); it != directors.end(); it++) {
		if (maxActors < it->second.actors.size()) {
			maxActors = it->second.actors.size();
			mInfluentionDirector = it->first;
		} else if (maxActors == it->second.actors.size() &&
					it->first < mInfluentionDirector) {
			mInfluentionDirector = it->first;
		}
	}

	return mInfluentionDirector;
}

std::string IMDb::get_best_year_for_category(std::string category) {
	if (!genres[category].empty()) {
		// de verificat daca merge

		int prevYear = 0, noFilmsInYear = 0, bestYear = 0, year;
		double maxRating = 0, ratingsSum = 0, averageRating = 0;
		time_t epoch;
		struct tm* date, storeDate;
		for(auto it = genres[category].begin(); it != genres[category].end(); it++) {
			// aflu anul
			epoch = movies[it->movie_id].timestamp;
			date = gmtime_r(&epoch, &storeDate);
			year = date->tm_year + 1900;

			if (prevYear == 0) {
				prevYear = year;
			} else if (year != prevYear) {  // daca s-a trecut la un an nou
				// fac average rating si compar cu maxRating de pana la punctul curent
				if (noFilmsInYear != 0)
					averageRating = 1.0 * ratingsSum / noFilmsInYear;
				else
					averageRating = 0;

				if (averageRating > maxRating) {
					maxRating = averageRating;
					bestYear = prevYear;
				} else if (averageRating == maxRating) {
					if (maxRating == 0 && noFilmsInYear != 0)
						bestYear = prevYear;
				}

				ratingsSum = 0;
				noFilmsInYear = 0;
				prevYear = year;
			}

			// aflu rating-ul filmului curent si adaug in suma de rating-uri
			if (get_rating(it->movie_id) != "none") {
				ratingsSum += stod(get_rating(it->movie_id));
				noFilmsInYear++;
			}
		}

		// mai trebuie sa verific o data, pentru ca la final ramane un
		// an neluat in considerare
		if (noFilmsInYear != 0) {
			averageRating = 1.0 * ratingsSum / noFilmsInYear;
			if (averageRating > maxRating) {
				bestYear = year;
			}
		}

		// daca nu exista filme cu rating
		if (bestYear == 0) {
			return "none";
		}

		return std::to_string(bestYear);
	}


	return "none";
}

std::string IMDb::get_2nd_degree_colleagues(std::string actor_id) {
	std::set<std::string> secondDegreeColleagues;
	std::string returnString;
	// parcurg toti colegii de gradul 1
	for (auto it = actors[actor_id].colleagues.begin();
			it != actors[actor_id].colleagues.end(); it++) {
		// parcurg toti colegii de gradul 1 al colegului de gradul 1
		for (auto it2 = actors[it->first].colleagues.begin();
				it2 != actors[it->first].colleagues.end(); it2++) {
			if (it == it2 || it2->first == actor_id)
				continue;

			// daca e de gradul 2 pentru primul
			if(actors[actor_id].colleagues.find(it2->first) ==
					actors[actor_id].colleagues.end()) {
				if (secondDegreeColleagues.empty()) {  // daca e gol vectorul
					secondDegreeColleagues.insert(it2->first);
				} else {
					if (find(secondDegreeColleagues.begin(), secondDegreeColleagues.end(),
						it2->first) == secondDegreeColleagues.end()) {  // daca nu se afla deja
						secondDegreeColleagues.insert(it2->first);
					}
				}
			}
		}
	}

	// daca s-a gasit cel putin un coleg de gradul 2
	if (!secondDegreeColleagues.empty()) {
		for (auto it = secondDegreeColleagues.begin();
				it != secondDegreeColleagues.end(); it++) {
			if (returnString.empty()) {
				returnString = *it;
			} else {
				returnString = returnString + " ";
				returnString = returnString + (*it);
			}
		}

		return returnString;
	// daca nu s-a gasit niciun coleg de gradul 2
	} else {
		return "none";
	}
}

std::string IMDb::get_top_k_most_recent_movies(int k) {
	if (movies.empty())
		return "none";

	int i = 0;
	std::string topMostRecent;
	// topMostRecent va fi sortat descrescator dupa timestamp, deci
	// trebuie sa plec de la sfarsit si iau ultimele k intrari
	for (auto it = topRecentMovies.rbegin(); it != topRecentMovies.rend(); it++) {
		if (i == 0) {
			topMostRecent = it->movie_id;
		} else {
			topMostRecent = topMostRecent + " ";
			topMostRecent = topMostRecent + it->movie_id;
		}

		i++;
		if (i == k)
			break;
	}

	return topMostRecent;
}

std::string IMDb::get_top_k_actor_pairs(int k) {
	// daca nu exista nicio pereche
	if (pairs.empty()) {
		return "none";
	} else {
		// daca s-a actualizat map-ul pairs
		if (pairsUpdated == true) {
			// se actualizeaza pairsTop
			pairsTop = flip_pairs(pairs);
			pairsUpdated = false;
		}

		int i = 0;
		std::string topPairs;
		// iau primele k perechi
		for (auto it = pairsTop.begin(); it != pairsTop.end(); it++) {
			if (topPairs.empty()) {
				topPairs += "(";
			} else {
				topPairs += " (";
			}

			topPairs += it->first.act1;
			topPairs += " ";
			topPairs += it->first.act2;
			topPairs += " ";
			topPairs += std::to_string(it->first.noMovies);
			topPairs += ")";

			i++;
			if (i == k)
				break;
		}

		return topPairs;
	}
}

std::string IMDb::get_top_k_partners_for_actor(int k, std::string actor_id) {
	auto cmp = [](std::pair <std::string, int> const &a,
						std::pair <std::string, int> const &b) {
		return a.second != b.second? a.second > b.second : a.first < b.first;
	};

	// fac o copie a vectorului de colegi ai actorului actor_id
	std::vector<std::pair<std::string, int> >
		collaborations(actors[actor_id].colleagues.begin(),
					actors[actor_id].colleagues.end());
	// il sortez descrescator dupa numarul de filme si crescator dupa id-uri
	sort(collaborations.begin(), collaborations.end(), cmp);
	std::string topPartners = "none";

	int i = 0;
	// pau primele k elemente
	for (auto it = collaborations.begin(); it != collaborations.end(); it++) {
		if (i == 0) {
			topPartners = it->first;
		} else {
			topPartners += " ";
			topPartners += it->first;
		}

		i++;

		if (i == k)
			break;
	}

	return topPartners;
}

std::string IMDb::get_top_k_most_popular_movies(int k) {
	// daca movies e gol
	if (movies.empty())
		return "none";

	// daca map-ul movies a fost actualizat
	if (moviesUpdated == true) {
		// actualizez mostPopularMovies
		mostPopularMovies = flip_movie(movies);
		moviesUpdated = false;
	}

	int i = 0;
	std::string topMostPopular;
	// iau primele k elemente din mostPopularMovies
	for (auto it = mostPopularMovies.begin();
			it != mostPopularMovies.end(); it++) {
		if (it->second == "") {
			continue;
		}

		i++;

		if (i == 1) {
			topMostPopular = it->second;
		} else {
			topMostPopular += " ";
			topMostPopular += it->second;
		}

		if (i == k)
			break;
	}

	return topMostPopular;
}

std::string IMDb::get_avg_rating_in_range(int start, int end) {
	int noRatings = 0;
	double sumRatings = 0, avgRating;

	// parcurg map-ul timestamps
	for (auto it = timestamps.begin();
			it != timestamps.end(); it++) {
		// sar peste intrarile cu timestamp < start
		if (it->first < start) {
			continue;
		}

		// daca s-a depasit end, ies din for
		if (it->first > end) {
			break;
		}

		if (get_rating(it->second) != "none") {
			sumRatings += stod(get_rating(it->second));
			noRatings++;
		}
	}

	if (noRatings == 0)
		return "none";

	avgRating = 1.0 * (sumRatings / noRatings);

	if ((int)avgRating == 10) {
		return std::to_string(1.0 * floor(100 * avgRating + 0.5)
																/ 100).substr(0, 5);
	}

	return std::to_string(1.00 * floor(100 * avgRating + 0.5)
															/ 100).substr(0, 4);
}

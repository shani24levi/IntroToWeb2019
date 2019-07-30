
#include <iostream>
#include<stdint.h>
#include <iostream>
#include "sqlDb.h"
#include<stdio.h>

#define HOSTNAME "localhost"
#define PORT 3306
#define USER "root"
#define PASSWORD "root"



/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>
/*
Include directly the different
headers from cppconn/ and mysql_driver.h + mysql_util.h
(and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <boost/tokenizer.hpp>
#include <string>

using namespace std;

typedef void(*func)();

static sql::Connection *con;
static vector<func> action_list = {};


static void print_possible_actions()
{
	vector<string> actions_collection = {
		"1.  How much albums recorded from date Y1 to date Y2",
		"2.  How much different songs a musician X recorded from date Y1 to date Y2",
		"3.  How much albums were a musician X involved from date Y1 to Y2",
		"4.  What is the most popular instrument from all over the years**",
		"5.  Get list of all instruments from album A",
		"6.  Get the most successfull from date Y1 to Y2 (largest number of albums)",
		"7.  Get the most popular manufacturer (the manufacturer whose instruments were involved in the most musical number)",
		"8.  How much musicians recorded over all the years",
		"9.  Who is the musician with the highest cooperations with other musicians",
		"10. What is the most popular janer",
		"11. Who is the recorder who has the largest number of songs from date Y1 to Y2",
		"12. What is the first album that was recorded in the studio",
		"13. List of all songs that included in 2 albums or more.",
		"14. List of all technicans that was involved in the recording of a full album",
		"15. who is the most liked musician (the one that recorded songs in the largest number of janers)",
		"16. Exit" };

	//system("cls");
	cout << "Menu" << endl << endl;

	for (size_t i = 0; i < 15; i++)
	{
		cout << actions_collection[i] << endl;
	}
	cout << endl << "Choose an option: ";
}

static int prompt_player_to_choose_action()
{
	int chosen_action = 0;

	print_possible_actions();
	cin >> chosen_action;

	return chosen_action;
}


void albums_recorded_from_to()
{
	string first_date, second_date;
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;

	cout << "Date Y1 (MM-DD-YYYY): ";
	cin >> first_date;

	cout << "Date Y2 (MM-DD-YYYY): ";
	cin >> second_date;
	cout << endl;

	//assuming albums were fully recorded in this period.
	pstmt = con->prepareStatement("SELECT count(*) as album_count FROM albums where albumRecordingBegining > '"+second_date+"' and albumRecordingEnd < '"+second_date+"';");
	res = pstmt->executeQuery();
	res->next();

	cout << res->getInt("album_count") << " albums were recorded.";
}

void musician_different_songs_from_to()
{
	string first_date, second_date;
	string musician_name = "";
	sql::ResultSet *res;
	sql::PreparedStatement *pstmt;

	cout << "Musician name: ";
	cin >> musician_name;
	cout << endl;

	cout << "Date Y1 (MM-DD-YYYY): ";
	cin >> first_date;

	cout << "Date Y2 (MM-DD-YYYY): ";
	cin >> second_date;


	pstmt = con->prepareStatement(
		"SELECT count(*) as num_of_songs FROM albums		  "
		"inner join musician_albums							  "
		"on musician_albums.albumId = albums.albumId		  "
		"inner join musicians								  "
		"on musicians.musicianId = musician_albums.musicianId "
		"inner join album_songs								  "
		"on album_songs.albumId = albums.albumId			  "
		"where musicianName = '"+musician_name+"' and albumRecordingBegining > '"+first_date+"' and albumRecordingEnd < '"+second_date+"' ");

	res = pstmt->executeQuery();
	res->next();

	cout << musician_name << " recorded " << res->getInt("num_of_songs") << " different songs.";
}

void musician_album_involvment_from_to()
{
	try{
		string first_date, second_date, sql_query;
		string musician_name = "";
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		cout << "Musician name: ";
		cin >> musician_name;
		cout << endl;

		cout << "Date Y1 (MM-DD-YYYY): ";
		cin >> first_date;
		cout << endl;

		cout << "Date Y2 (MM-DD-YYYY): ";
		cin >> second_date;
		cout << endl;

		sql_query =
			"Select COUNT(*) as count"
			" FROM albums"
			" INNER JOIN musician_albums"
			" ON albums.albumId = musician_albums.albumId"
			" INNER JOIN musicians"
			" ON musicians.musicianId = musician_albums.musicianId"
			" WHERE(Select musicianId from musicians WHERE"
			" musicians.musicianName = '" + musician_name + "') = musicians.musicianId and albumRecordingEnd > '" + first_date + "' and albumRecordingEnd < '" + second_date + "'";

		pstmt = con->prepareStatement(sql_query);//fill
		res =  pstmt->executeQuery();
		res->next();

		cout << musician_name << " was involved in " << (*res).getInt("count") << ".";
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

void most_popular_instrument_of_all_time()
{
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	pstmt = con->prepareStatement(																									
		"select instruments.instrumentName from musical_numbers																		"
		"inner join musical_numbers_instruments on musical_numbers.musicalNumberId = musical_numbers_instruments.musicalNumberId	"
		"inner join instruments on instruments.instrumentId = musical_numbers_instruments.instruemntId								"
		"group by instruments.instrumentId																							"
		"limit 1;																													"
	);
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("instrumentName") << " is the most popular instrument.";//fill
}

void count_instrument_in_album()
{
	string album_name;
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;

	cout << "album name: ";
	cin >> album_name;
	cout << endl;

	pstmt = con->prepareStatement(
		"select instrumentName from instruments																	"
		"inner join albums_instruments on instruments.instrumentId = albums_instruments.instrumentId		    "
		"where albums_instruments.albumId = (select albums.albumId from albums where albums.albumName = '"+album_name+"');"
	);
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("instrumentName") << " is the most popular instrument.";//fill
}

void most_successful_producer()
{
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;

	pstmt = con->prepareStatement(                        
	"Select producerName, max(reps) from                  "
	"(Select producerName, count(albumId) as reps         "
	"FROM producers                                       "
	"INNER JOIN producer_albums                           "
	"ON producer_albums.producerId = producers.producerId "
	"group by producer_albums.producerId ) as temp_table  "
	);
	res = pstmt->executeQuery();
	res->next();

	cout << "most successful producer is " << (*res).getString("producerName");
}

void most_popular_manufacturer()
{
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;

	pstmt = con->prepareStatement(

		"select instruments.instrumentMaker from instruments																		   "
		"where instruments.instrumentName = (select instruments.instrumentName from musical_numbers									   "
		"inner join musical_numbers_instruments on musical_numbers.musicalNumberId = musical_numbers_instruments.musicalNumberId	   "
		"inner join instruments on instruments.instrumentId = musical_numbers_instruments.instruemntId								   "
		"group by instruments.instrumentId																							   "
		"limit 1);																													   "
	);
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("instrumentMaker") << " is the most popular manufacturer .";
}

void how_much_musicians()
{
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;

	pstmt = con->prepareStatement("Select count(*) as count1 from musicians");
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getInt("count1") << " musicians played over the years";
}

void most_cooperated_musician()
{
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;

	pstmt = con->prepareStatement(
	"select musicianName, max(participants) from                                         "
	"(Select musicians.musicianName,albumId,count(*) as participants from musicians      "
	"INNER JOIN musician_albums                                                          "
	"ON musicians.musicianId = musician_albums.musicianId                                "
	"group by albumId) as tablu                                                          "
	);
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("musicianName") << " is the most cooperated musician";
}

void most_popular_genre()
{
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;

	pstmt = con->prepareStatement(
		"select songGenre, max(rep) as rep from(Select*, count(*) as rep from songs	  "
		"group by songGenre) as table1												  "
	);
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("songGenre") << " is the most popular genre";//fill
}

void most_successful_recorder()
{
	string first_date, second_date;
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;
	

	cout << "Date Y1 (MM-DD-YYYY): ";
	cin >> first_date;

	cout << "Date Y2 (MM-DD-YYYY): ";
	cin >> second_date;


	pstmt = con->prepareStatement(
		"SELECT*, count(*) as songs FROM songs												  "
		"inner join recorders																  "
		"on songs.recorderId = recorders.recorderId											  "
		"inner join album_songs																  "
		"on album_songs.songId = songs.songId												  "
		"inner join albums																	  "
		"on albums.albumId = album_songs.albumId											  "
		"where albumRecordingBegining > '"+ first_date +"' and albumRecordingEnd < '" + second_date + "'	  "
		"group by recorderName																  "
		"order by songs DESC																  "
		"limit 1																			  "
	);
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("recorderName") << " is the most successfull recorder";
}

void first_album()
{
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	pstmt = con->prepareStatement("SELECT * FROM albums order by albumRecordingEnd ASC");
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("albumName") << " is the first album";
}

void list_of_all_songs_in_2_albums_or_more()
{
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	pstmt = con->prepareStatement(
		"Select*, count(*) as rep from album_songs		  "
		"inner join songs								  "
		"on album_songs.songId = songs.songId			  "
		"group by songName								  "
		"having rep > 1									  "
	);
	res = pstmt->executeQuery();
	
	res->beforeFirst();
	while (res->next())
	{
		cout << (*res).getString("songName") << " exist in more than 2 albums.";
		cout << endl;
	}
}

void recorders_who_recorded_full_album()
{
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	pstmt = con->prepareStatement(
		"SELECT *, count(*) as number_of_tracks FROM recorders	"
		"inner join songs										"
		"on recorders.recorderId = songs.recorderId				"
		"inner join album_songs									"
		"on album_songs.songId = songs.songId					"
		"inner join albums										"
		"on albums.albumId = album_songs.albumId				"
		"group by albums.albumId								"
		"having albums.albumNumberOfTracks = number_of_tracks	"
	);
	res = pstmt->executeQuery();

	res->beforeFirst();
	while (res->next())
	{
		cout << (*res).getString("recorderName") << " recorded full album.";
		cout << endl;
	}
}

void most_genre_wide_musician()
{
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	pstmt = con->prepareStatement(															   
		"select musicians.musicianName from musicians										   "
		"inner join musician_songs on musicians.musicianId = musician_songs.musicianId		   "
		"inner join songs on songs.songId = musician_songs.songId							   "
		"group by musicians.musicianName													   "
		"order by count(songs.songGenre) desc												   "
		"limit 1;																			   "
	);
	res = pstmt->executeQuery();
	res->next();

	cout << (*res).getString("musicianName") << " is the most genre wide musician .";
}


sql::Connection* init_connection(string hostname, int port,string user,string password)
{
	try {
		sql::Connection *con;
		sql::Driver *driver;
		sql::ConnectOptionsMap connection_properties;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		return con;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return NULL;
	}
}


int main(int argc, char *argv[])
{
	action_list.push_back(albums_recorded_from_to);//1V
	action_list.push_back(musician_different_songs_from_to);//2V
	action_list.push_back(musician_album_involvment_from_to);//3V
	action_list.push_back(most_popular_instrument_of_all_time);//4V
	action_list.push_back(count_instrument_in_album);//5
	action_list.push_back(most_successful_producer);//6V
	action_list.push_back(most_popular_manufacturer);//7
	action_list.push_back(how_much_musicians);//8V
	action_list.push_back(most_cooperated_musician);//9V
	action_list.push_back(most_popular_genre);//10V
	action_list.push_back(most_successful_recorder);//11V
	action_list.push_back(first_album);//12V
	action_list.push_back(list_of_all_songs_in_2_albums_or_more);//13
	action_list.push_back(recorders_who_recorded_full_album);//14V
	action_list.push_back(most_genre_wide_musician);//15V
	
	//3 more missing

	try {
		int running_index = 0;

		con = init_connection(HOSTNAME, PORT, USER, PASSWORD);
		con->setSchema("newschema");

		while (running_index != 99)
		{
			int chosen_action;
			chosen_action = prompt_player_to_choose_action();

			if (chosen_action <= 16 && chosen_action > 0)
			{
				action_list[chosen_action - 1]();
				cin.get();
			}
		}

	}
	catch (sql::SQLException &e) {

	}

	std::cout << endl;
	return EXIT_SUCCESS;
}

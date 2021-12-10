/*
	Simple udp client
*/
#define NOMINMAX

#include <iostream>
#include <memory>
#include <chrono>
#include "UdpSocket.h"
#include "../GameState/GameState.h"
#include <SFML/Graphics.hpp>


//ip address of udp server
std::string const kIpAddr = "127.0.0.1";
//The port on which to listen for incoming data
u_short const kPort = 8888;
size_t const kBufferSize = 512;
char buffer[kBufferSize];
bool firstEnter = 1;


void sleep(unsigned long us)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto finish = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
	while (microseconds.count() < us)
	{
		finish = std::chrono::high_resolution_clock::now();
		microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
	}
}

int main(int argc, const char* argv[])
{
	std::unique_ptr<UdpSocket> sock_ptr;
	std::string name = "Name";
	std::cout << "Enter name:\n";
	std::cin >> name;
	GameState state;

	try
	{
		sock_ptr = std::make_unique<UdpSocket>(kIpAddr, kPort);
	}
	catch (std::exception const& err)
	{
		std::cout << "Couldn't init socket: " << err.what() << "\n";
		exit(EXIT_FAILURE);
	}

	//start communication
	//send the message
	if (sock_ptr->send(name.c_str(), name.length()) != 0)
	{
		std::cout << "Failed to send\n";
		exit(EXIT_FAILURE);
	}

	std::cout << "request sent\n";

	bool blockChanged = 0;
	PlayerPos blockPos;

	int i = 0;
	sf::RenderWindow window(sf::VideoMode(600, 300), "Perfect Game");
	
	sf::Texture TerainT;
	TerainT.loadFromFile("Terain.png");

	sf::Texture AirT;
	AirT.loadFromFile("Air.png");

	sf::Texture PlayerT;
	PlayerT.loadFromFile("Player.png");

	sf::Sprite PlayerS;
	sf::Sprite BackS;
	sf::Sprite PersonS;
	PersonS.setTexture(PlayerT);

	
	sf::Font font;
	font.loadFromFile("arial.ttf");

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(12);
	text.setFillColor(sf::Color::Black);
	//text.setStyle(sf::Text::Bold);
	
	GameState::Block block = GameState::Block::Ground;

	PlayerPos client_pos;

	while (1)
	{
		//receive a reply and print it
		size_t sz = kBufferSize;
		if (sock_ptr->recv(buffer, sz) != 0)
		{
			std::cout << "No data to recv\n";
			sleep(1e5);
			continue;
			//exit(EXIT_FAILURE);
		}

		//std::cout << "Received game state: " << sz << "\n";
		state.deserialize(buffer, sz);
		Player* p = state.getPlayer(name);
		PlayerPos curr_pos = p->getPos();
		std::cout << "Received player pos: (";
		std::cout << (int)curr_pos.first << "," << (int)curr_pos.second << ")\n";
		
		
		if (firstEnter)
		{
			client_pos = curr_pos;
			firstEnter = 0;
		}
		PlayerS.setPosition(sf::Vector2f(30.f * client_pos.first, 30.f * client_pos.second));
		PlayerS.setTexture(PlayerT);

		text.setPosition(30.f * client_pos.first, 30.f * client_pos.second);
		text.setString(name);
		
		
		
		for (int i = 0; i < state.getRows(); i++)
		{
			for (int j = 0; j < state.getCols(); j++)
			{
				if (state.getMap()[i][j] == GameState::Block::Background)
					BackS.setTexture(AirT);
				else if (state.getMap()[i][j] == GameState::Block::Ground)
					BackS.setTexture(TerainT);

				BackS.setPosition(sf::Vector2f(j * 30.f, i * 30.f));

				window.draw(BackS);
			}
		}
		

		for (auto &it : state.getPlayers())
		{
			if (it.first != name)
			{
				PersonS.setPosition(it.second.getPos().first * 30.f, it.second.getPos().second * 30.f);
				text.setPosition(it.second.getPos().first * 30.f, it.second.getPos().second * 30.f);
				text.setString(it.first);
				window.draw(PersonS);
				
				window.draw(text);
			}
		}
		
		
		
		
		
		
		sf::Event event;

		window.pollEvent(event);

		if (event.type == sf::Event::Closed)
		{
			window.close();
			std::cout << "Exit\n";
			break;
		}
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::D)
			{
				client_pos.first++;
				client_pos.first = client_pos.first % state.getCols();
				PlayerS.setTexture(PlayerT);
				
			}
			if (event.key.code == sf::Keyboard::A)
			{
				client_pos.first--;
				client_pos.first = client_pos.first % state.getCols();
				PlayerS.setTexture(PlayerT);

			}
			if (event.key.code == sf::Keyboard::W)
			{
				client_pos.second--;
				client_pos.second = client_pos.second % state.getRows();
				PlayerS.setTexture(PlayerT);

			}
			if (event.key.code == sf::Keyboard::S)
			{
				client_pos.second++;
				client_pos.second = client_pos.second % state.getRows();
				PlayerS.setTexture(PlayerT);

			}
			if (event.key.code == sf::Keyboard::E)
			{
				if (state.getMap()[client_pos.second][client_pos.first + 1] == GameState::Block::Background)
				{
					blockChanged = 1;
					blockPos.first = client_pos.first + 1;
					blockPos.second = client_pos.second;
					block = GameState::Block::Ground;
				}
				else if (state.getMap()[client_pos.second][client_pos.first + 1] == GameState::Block::Ground)
				{
					blockChanged = 1;
					blockPos.first = client_pos.first + 1;
					blockPos.second = client_pos.second;
					block = GameState::Block::Background;
				}

			}
			if (event.key.code == sf::Keyboard::Q)
			{
				if (state.getMap()[client_pos.second][client_pos.first - 1] == GameState::Block::Background)
				{
					blockChanged = 1;
					blockPos.first = client_pos.first - 1;
					blockPos.second = client_pos.second;
					block = GameState::Block::Ground;
				}
				else if (state.getMap()[client_pos.second][client_pos.first - 1] == GameState::Block::Ground)
				{
					blockChanged = 1;
					blockPos.first = client_pos.first - 1;
					blockPos.second = client_pos.second;
					block = GameState::Block::Background;
				}

			}
		}
		




		text.setPosition(30.f * client_pos.first, 30.f * client_pos.second);
		text.setString(name);
		window.draw(PlayerS);
		window.draw(text);


		window.display();

		curr_pos = client_pos;
		p->updatePos(curr_pos.first, curr_pos.second);
		sz = kBufferSize;

		if (blockChanged)
		{
			p->serialize(buffer, sz, blockPos.second , blockPos.first, (char)block);
			blockChanged = 0;
		}
		else
			p->serialize(buffer, sz);

		
		
		

		if (sock_ptr->send(buffer, sz) != 0)
		{
			std::cout << "Failed to send pos\n";
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
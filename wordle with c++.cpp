#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>

#define RESET_TEXT "\033[0m"

#include <windows.h>

using namespace std;
using namespace std::chrono;

struct Player {
    string name;
    int score;
    int timeTaken; // Time taken in seconds

    bool operator<(const Player &other) const {
        return score > other.score; // Sort in descending order by score
    }
};

// Base class for common game functionalities
class Game {
public:
    Game() {
        srand(static_cast<unsigned int>(time(0)));
    }

    void setConsoleColor(int textColor) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, textColor);
    }

    void displayWelcomeText() {
        cout << "\n\n";
        cout << "W       W  EEEEEEE  L         CCCCCCC   OOOOOOO   M       M  EEEEEEE\n";
        cout << "W       W  E        L        C         O       O  MM     MM  E      \n";
        cout << "W   W   W  EEEEE    L        C         O       O  M M   M M  EEEEE  \n";
        cout << " W W W W   E        L        C         O       O  M  M M  M  E      \n";
        cout << "  W   W    EEEEEEE  LLLLLLL   CCCCCCC   OOOOOOO   M   M   M  EEEEEEE\n\n";

        cout << "Press any key to continue...\n";
        cin.ignore(); // Clear newline character left in the input buffer
        cin.get();    // Wait for user input to continue
    }

    void displayRules() {
        clearConsole();
        cout << "The rules are:\n\n";
        cout << "1. You have to guess the Wordle in six tries or less.\n";
        cout << "2. Each word you enter must be a valid five-letter word from the word list.\n";
        cout << "3. Correct letters in the correct position are displayed in green.\n";
        cout << "4. Correct letters in the wrong position are displayed in yellow.\n";
        cout << "5. Incorrect letters are displayed as '_'.\n";
        cout << "6. Answers are never plurals.\n\n";

        cout << "Leaderboard Rules:\n";
        cout << "1. Your score starts at 1500 and decreases based on incorrect guesses and misplaced letters.\n";
        cout << "2. The top 5 highest scores are saved to the leaderboard.\n";
        cout << "3. If a player with the same name scores higher than their previous record, the new score will replace the old one.\n";
        cout << "4. Only the best score for each player is shown on the leaderboard, and no duplicate names are allowed.\n\n";

        cout << "Press any key to start the game.\n";

        cin.get();    // Wait for user input to continue
    }

protected:
    void clearConsole() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
};

// Derived class for Wordle-specific functionalities
class WordleGame : public Game {
public:
    WordleGame() {
        loadWords();
        loadLeaderboard();
    }

    ~WordleGame() {
        delete[] words;  // Free dynamically allocated memory
    }

    void start() {
        clearConsole();
        displayRules();

        while (true) {
            clearConsole();
            string* targetWord = new string(getRandomWord()); // Dynamic allocation for target word
            string guessedWord;
            bool wordGuessed = false;
            int score = 1500; // Initialize score

            auto gameStartTime = steady_clock::now(); // Record game start time

            cout << "Enter your name: ";
            string playerName;
            cin >> playerName;

            for (int attempt = 0; attempt < 6; ) {  // Only increment if the word is valid
                cout << "\n\nAttempt " << attempt + 1 << ": Enter a five-letter word: ";
                cin >> guessedWord;
                transformToUpper(guessedWord);

                if (guessedWord.length() != 5) {
                    cout << "The length of the word should be 5.\n\n";
                    continue;  // Give the user another chance
                }

                if (isWordInDictionary(guessedWord)) {
                    if (guessedWord == *targetWord) {
                        cout << "Yay! You guessed the word correctly.\n\n";
                        wordGuessed = true;
                        break;
                    } else {
                        provideFeedback(guessedWord, *targetWord, score);
                        cout << "Current Score: " << score << "\n";
                        ++attempt;  // Increment only if the word is valid
                    }
                } else {
                    cout << "The word is not in the dictionary.\n\n";
                    continue;  // Give the user another chance
                }

                auto currentTime = steady_clock::now();
                auto totalTimeTaken = duration_cast<seconds>(currentTime - gameStartTime).count();

                cout << "Time: " << totalTimeTaken << " seconds\n";
            }

            auto finalTime = steady_clock::now();
            auto totalTimeTaken = duration_cast<seconds>(finalTime - gameStartTime).count();

            if (!wordGuessed) {
                cout << "\nSadly, you couldn't guess the word correctly. \nThe word was " << *targetWord << ".\n";
            }

            cout << "Final Score: " << score << "\n";
            cout << "Total time taken: " << totalTimeTaken << " seconds\n";

            // Save player info to leaderboard
            savePlayerToLeaderboard({playerName, score, totalTimeTaken});

            // Display the leaderboard
            displayLeaderboard();

            cout << "\nPress any key to play again or 'q' to quit: ";
            char choice;
            cin >> choice;
            if (choice == 'q' || choice == 'Q') break;

            delete targetWord; // Free dynamically allocated memory
        }
    }

private:
    string* words = nullptr;  // Pointer for dynamic array of words
    size_t wordCount = 0;     // To keep track of word count
    vector<Player> leaderboard; // Vector to store leaderboard data

    void loadWords() {
        ifstream file("all.txt");
        if (!file.is_open()) {
            cerr << "Error opening dictionary file.\n";
            exit(EXIT_FAILURE);
        }

        vector<string> tempWords;
        string word;
        while (getline(file, word)) {
            word.erase(remove(word.begin(), word.end(), '\n'), word.end());
            tempWords.push_back(word);
        }

        wordCount = tempWords.size();
        words = new string[wordCount];  // Allocate memory for words dynamically
        copy(tempWords.begin(), tempWords.end(), words);
    }

    string getRandomWord() {
        int randomIndex = rand() % wordCount;
        return words[randomIndex];
    }

    void transformToUpper(string &str) {
        for (char &c : str) {
            c = toupper(c);
        }
    }

    bool isWordInDictionary(const string &word) {
        return find(words, words + wordCount, word) != words + wordCount;
    }

    void provideFeedback(const string &guess, const string &target, int &score) {
        for (size_t i = 0; i < guess.length(); ++i) {
            if (guess[i] == target[i]) {
                setConsoleColor(10);
                cout << guess[i] << '\t';
                setConsoleColor(7);
            } else if (target.find(guess[i]) != string::npos) {
                setConsoleColor(14);
                cout << guess[i] << '\t';
                setConsoleColor(7);
                score -= 25;  // Deduct 25 points for a misplaced letter
            } else {
                cout << "_\t";
                score -= 50;  // Deduct 50 points for an incorrect letter
            }
        }
        cout << '\n';
    }

    void savePlayerToLeaderboard(const Player &player) {
        // Check if the player already exists in the leaderboard
        auto it = find_if(leaderboard.begin(), leaderboard.end(), [&player](const Player &p) {
            return p.name == player.name;
        });

        if (it != leaderboard.end()) {
            // If player exists, keep the higher score
            if (player.score > it->score) {
                *it = player;  // Update with the new higher score and time
            }
        } else {
            // If player does not exist, add the player to the leaderboard
            leaderboard.push_back(player);
        }

        // Sort leaderboard and keep only the top 5 players
        sort(leaderboard.begin(), leaderboard.end());
        if (leaderboard.size() > 5) {
            leaderboard.resize(5);
        }

        // Save leaderboard to file
        ofstream outFile("leaderboard.txt");
        if (outFile.is_open()) {
            for (const auto &entry : leaderboard) {
                outFile << entry.name << " " << entry.score << " " << entry.timeTaken << "\n";
            }
            outFile.close();
        }
    }

    void loadLeaderboard() {
        ifstream inFile("leaderboard.txt");
        if (inFile.is_open()) {
            Player player;
            while (inFile >> player.name >> player.score >> player.timeTaken) {
                leaderboard.push_back(player);
            }
            inFile.close();
        }

        // Sort leaderboard and keep only the top 5 players
        sort(leaderboard.begin(), leaderboard.end());
        if (leaderboard.size() > 5) {
            leaderboard.resize(5);
        }
    }

    void displayLeaderboard() {
        cout << "\n--Leaderboard--\n";
        cout << left << setw(15) << "Name" << setw(10) << "Score" << setw(10) << "Time (s)" << "\n";
        for (const auto &player : leaderboard) {
            cout << left << setw(15) << player.name << setw(10) << player.score << setw(10) << player.timeTaken << "\n";
        }
    }
};

int main() {
    WordleGame game;
    game.displayWelcomeText();
    game.start();
    return 0;
}

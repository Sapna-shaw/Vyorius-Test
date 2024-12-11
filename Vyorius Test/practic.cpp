#include <bits/stdc++.h>
using namespace std;

// Function to load the ratings matrix from a CSV file
vector<vector<int>> loadRatingsMatrix(const string &filename) {
    vector<vector<int>> matrix;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file." << endl;
        return matrix;
    }

    string line;
    while (getline(file, line)) {
        vector<int> row;
        stringstream ss(line);
        string cell;
        while (getline(ss, cell, ',')) {
            row.push_back(stoi(cell));
        }
        matrix.push_back(row);
    }

    file.close();
    return matrix;
}

// Function to load movie names from a CSV file
vector<string> loadMovieNames(const string &filename) {
    vector<string> movieNames;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file." << endl;
        return movieNames;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string id, name;
        getline(ss, id, ',');
        getline(ss, name, ',');
        movieNames.push_back(name);
    }

    file.close();
    return movieNames;
}

// Function to calculate Pearson correlation between two users
double calculatePearsonSimilarity(const vector<int> &user1, const vector<int> &user2) {
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    int count = 0;

    for (size_t i = 0; i < user1.size(); ++i) {
        if (user1[i] != 0 && user2[i] != 0) {
            sumX += user1[i];
            sumY += user2[i];
            sumXY += user1[i] * user2[i];
            sumX2 += user1[i] * user1[i];
            sumY2 += user2[i] * user2[i];
            count++;
        }
    }

    if (count == 0) return 0.0; // No common ratings to compute correlation

    double numerator = sumXY - (sumX * sumY / count);
    double denominator = sqrt((sumX2 - sumX * sumX / count) * (sumY2 - sumY * sumY / count));
    
    if (denominator == 0) return 0.0;
    return numerator / denominator;
}

// Function to predict ratings for a specific user
vector<pair<int, double>> predictRatings(const vector<vector<int>> &matrix, int userIndex) {
    vector<pair<int, double>> predictions;
    const vector<int> &targetUser = matrix[userIndex];

    // Calculate the global average movie ratings
    vector<double> globalAvgRatings(matrix[0].size(), 0);
    for (size_t movie = 0; movie < globalAvgRatings.size(); ++movie) {
        double sum = 0.0;
        int count = 0;
        for (size_t user = 0; user < matrix.size(); ++user) {
            if (matrix[user][movie] != 0) {
                sum += matrix[user][movie];
                count++;
            }
        }
        if (count > 0) {
            globalAvgRatings[movie] = sum / count;
        }
    }

    // Predict ratings for unrated movies (those with value 0 in targetUser)
    for (size_t movie = 0; movie < targetUser.size(); ++movie) {
        if (targetUser[movie] != 0) continue;  // Skip already rated movies

        double weightedSum = 0.0, similaritySum = 0.0;
        for (size_t otherUser = 0; otherUser < matrix.size(); ++otherUser) {
            if (otherUser == userIndex) continue;

            double similarity = calculatePearsonSimilarity(targetUser, matrix[otherUser]);
            if (similarity < 0.1) continue;  // Ignore users with very low similarity
            
            double rating = matrix[otherUser][movie];
            weightedSum += similarity * rating;
            similaritySum += fabs(similarity);
        }

        // Use global average as fallback if no similar users
        double predictedRating = (similaritySum == 0) ? globalAvgRatings[movie] : (weightedSum / similaritySum);
        predictions.emplace_back(movie, predictedRating);
    }

    return predictions;
}

// Function to recommend top N movies
vector<int> recommendMovies(const vector<pair<int, double>> &predictions, int N, const vector<int>& userRatings) {
    vector<pair<int, double>> sortedPredictions = predictions;
    
    // Sort the predictions by predicted rating (descending)
    sort(sortedPredictions.begin(), sortedPredictions.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
        return b.second > a.second;
    });

    vector<int> recommendations;
    int count = 0;
    for (const auto &prediction : sortedPredictions) {
        int movieIndex = prediction.first;
        
        // Only recommend movies that the user hasn't rated yet
        if (userRatings[movieIndex] == 0 && count < N) {
            recommendations.push_back(movieIndex);
            count++;
        }
    }

    return recommendations;
}

int main() {
    string ratingsFile = "ratings.csv";  // Path to your ratings CSV
    string moviesFile = "movies.csv";    // Path to your movies CSV

    vector<vector<int>> ratingsMatrix = loadRatingsMatrix(ratingsFile);
    vector<string> movieNames = loadMovieNames(moviesFile);

    if (ratingsMatrix.empty()) {
        cerr << "Error: Ratings matrix is empty." << endl;
        return 1;
    }

    if (movieNames.empty()) {
        cerr << "Error: Movie names list is empty." << endl;
        return 1;
    }

    int userIndex;
    cout << "Enter the user index (0-based): ";
    cin >> userIndex;

    // Get predicted ratings for unrated movies
    vector<pair<int, double>> predictions = predictRatings(ratingsMatrix, userIndex);

    cout << "\nPredicted ratings for unrated movies for User " << userIndex << ":\n";
    for (const auto &prediction : predictions) {
        int movieIndex = prediction.first;
        double predictedRating = prediction.second;
        cout << "Movie: " << movieNames[movieIndex] << " | Predicted Rating: " << predictedRating << endl;
    }

    return 0;
}

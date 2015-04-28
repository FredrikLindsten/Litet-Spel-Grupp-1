#include "LevelParser.h"


LevelParser::LevelParser ( AssetManager* assetManager ) 
{
	if (assetManager == nullptr)
	{
		OutputDebugString ( "Error in LevelParser constructor: assetManager == nullptr" );
	}
	else
	{
		this->assetManager = assetManager;
	}
	assetUtility::fileToStrings ( "Levels.txt", levelNames );
}


LevelParser::~LevelParser ( ) 
{

}

Level LevelParser::LoadLevel ( int levelID ) 
{
	Level level = Level();

	if ( levelID <= 0 || levelID > levelNames.size ( ) - 1 ) {
		OutputDebugString ( "Error on LoadLevel: levelID out of bounds." );
		return level;
	}

	std::string pathToLevel = "Levels/" + levelNames.at ( levelID );
	std::vector<std::string> unparsedLevel;
	assetUtility::fileToStrings ( pathToLevel, unparsedLevel );

	std::vector<std::string> unparsedLine;
	for each ( std::string line in unparsedLevel)
	{
		unparsedLine.clear ( );
		assetUtility::splitStringToVector ( line, unparsedLine, "," );

		//TODO Parse the GameObjects

	}
	return level;
}
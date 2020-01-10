#include <string>
#include <iostream>
#include <algorithm>
#include <random>

/*
Dragonscar Keep v. 0.01

Edit Log:
		Created: 12/23/19
			Transferral from earlier format begun, revising code along the way. Not complete; removed old code from repl.it as transferred (not up to date), and began reworking on program flow. Removed unnecessary declarations from program.
			Future notes: Continue building out flow, starting with menuCreation to begin the character creation, as well as serving as the return point from a new game.
		Continued: 12/24/19
			Confirmed debug of code-to-date, as well as continuing with rewrite of code, starting with menuCreation().
			Split combat into menu and action, allowing for attacks to happen simultaneously, while actions are selected in advance.
			Spent an inordinate amount of time trying to fix a 'bug'. Missed break;s.
		Continued: 12/26/19
			Changed fucntions to have variables declared in the body, rather than in the initial declaration (unless is an input). Considered (and roughly implemented) a split for the class to deliniate all of the information; reverted due to unsure if helpful in any way.
			Created initial equipment system, calling variable names instead of hard-coding.
		Continued: 12/27/19
			Integrated series of class definitions again, attempting to deperate functions. Could not get variables to access by friends, so scrapped back into a single class for entities.
			Created system to create maps of variable size, as well as printing them as part of the HUD.
			Began working on system to check for open areas in areas around the current location, in order to be able to randomly expand outward with further functions.
		Continued: 01/10/20
			Developed navigation system (roughly), allowing player positions to update and map to reflect.
			Developed system to 'populate' room, checking for combat chance.
*/

int roll(int lower, int upper);
int exit();
int characterLocationX, characterLocationY, exitCount{ 0 };
bool clearNorth, clearSouth, clearEast, clearWest;
int armor[4][2]{ {0,0}, {2,0}, {3,0}, {4,0} }, weapon[4][3]{ {4,0,0}, {6,1,0}, {6,2,0}, {8,1,0} };
int mapSizeX{ 7 }, mapSizeY{ 7 };
std::string map[11][11]{ {" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "},{" "," "," "," "," "," "," "," "," "," "," "} };
std::string armorName[4]{ " tattered rags",	" battered leather hauberk"," chipped chainmail",	" dented breastplate" };
std::string weaponName[4]{ " bare fists",	" rusted dagger",			" bent shortsword",		" chipped longsword" };

void menuMain(), menuCreation(), menuCombat(), navigation(), promptNavigation();
void checkCreation(), pause(), confirmQuit(), combat(), combatCleanup(), opponentGen(), playerAction(), opponentAction(), creation(), transition(int direction), drawRoom(), combatWin();
void mapGen(int xAxis, int yAxis), drawBorders(int xAxis, int yAxis), drawMap(int xAxis, int yAxis), directionCheck(), checkNorth(), checkSouth(), checkEast(), checkWest(), callExits(), roomCheck();

class entity {
protected:
	int strength{ 10 }, dexterity{ 10 }, constitution{ 10 }, intelligence{ 10 }, wisdom{ 10 }, charisma{ 10 };
	int strMod{ 0 }, dexMod{ 0 }, conMod{ 0 }, intMod{ 0 }, wisMod{ 0 }, chaMod{ 0 };
	int callMod(int input), statRoll();

	int armorBase{ 0 }, attackBase{ 0 }, damageBase{ 0 }, healthMax{ 0 }, flightBase{ 0 }, initiativeBase{ 0 };
	int itemWeaponBase{ 0 }, itemWeaponAttack{ 0 }, itemWeaponMagic{ 0 };
	int itemArmorBonus{ 0 }, itemArmorMagic{ 0 };
	int armorEquipped{ 0 }, weaponEquipped{ 0 };
	void menuEquipWeapon(), menuEquipArmor();

	int level{ 1 }, experienceCurrent{ 0 }, experienceToLevel{ 1000 };
	void levelCheck(), levelUp();
public:
	std::string name;

	int armorCurrent{ 0 }, attackCurrent{ 0 }, damageCurrent{ 0 }, healthCurrent{ 0 }, flightCurrent{ 0 }, initiativeCurrent{ 0 };
	int callDamage(int input), callHealing(int input), attackRoll(), damageRoll();
	void reset(), HUD();

	void checkEquipment(), inventory();

	int callExperience(int input);

	bool deathFlag{ false }, flightFlag{ false }, killFlag{ false };
	bool hasArmor[4]{ true, false, false, false }, hasWeapon[4]{ true, false, false, false };
	bool attack{ false }, rest{ false }, reckless{ false }, cautious{ false }, flee{ false };

	void introducePlayer(), entityRoll();
};
	int entity::statRoll() { // Function to roll for the stats; roll 4d6, dropping lowest.
		int output;
		int rolls[4]{ roll(1,6), roll(1,6), roll(1,6), roll(1,6) };
		std::sort(rolls, rolls + 4);
		output = rolls[1] + rolls[2] + rolls[3];
		return output;
	};
	int entity::callMod(int input) { // Function to create the modifiers for the stats upon calling.
		int output;
		output = ((input - 10) / 2);
		return output;
	}
	void entity::reset() { 	// Function to restore all temporary, public variables to their default values, removing any modifiers.
	armorCurrent = armorBase;
	attackCurrent = attackBase;
	damageCurrent = damageBase;
	initiativeCurrent = initiativeBase;
	deathFlag = false;
	flightFlag = false;
	attack = false;
	rest = false;
	reckless = false;
	cautious = false;
	flee = false;
};
	void entity::HUD() { // Function to report entity's health.
		std::cout << std::endl;
		std::cout << "HP: " << healthCurrent << " / " << healthMax << "   ||  " << weaponName[weaponEquipped] << " |" << armorName[armorEquipped] << std::endl;
};
	int entity::callDamage(int input) { // Function to deal damage to Entity called within; first, checks that damage is not less than 1, before removing health from current. If below 0 triggered the deathFlag state.
		int output;
		if (input < 1) {
			input = 1;
		}
		healthCurrent = healthCurrent - input;
		if (healthCurrent <= 0) {
			deathFlag = true;
		};
		output = input;
		return output;
	}
	int entity::callHealing(int input) { // Function to restore health to Entity called from; first, confirms that damage is not less than 0, before restoring health and capping to the Max.
		int output;
		if (input < 0) {
			input = 0;
		}
		healthCurrent = healthCurrent + input;
		if (healthCurrent > healthMax) {
			healthCurrent = healthMax;
		}
		output = input;
		return output;
	}
	int entity::attackRoll() { // Function to roll for attack automatically, adding all relevant modifiers.
		int output;
		output = roll(1, 20) + attackCurrent + itemWeaponAttack;
		return output;
	};
	int entity::damageRoll() { // Function to roll for damage, with all relevant modifiers. Ensures that damage + modifiers is not less than 1, to double-check for callDamage.
		int output;
		output = roll(1, itemWeaponBase) + damageCurrent;
		if (output < 1) {
			output = 1;
		}
		return output;
	}
	void entity::checkEquipment() {
		itemArmorBonus = armor[armorEquipped][0];
		itemWeaponBase = weapon[weaponEquipped][0];
		itemWeaponAttack = weapon[weaponEquipped][1];
	}
	void entity::inventory() {
		int response, choice;
		std::cout << "You are currently wielding:" << weaponName[weaponEquipped] << "\nYou are currently wearing:" << armorName[armorEquipped] << std::endl;
		std::cout << "\n\n\n\n\n\n\n\nWould you like to equip a weapon (1) or armor (2) (0 to return)?";
		std::cin >> response;
		std::cin.clear();
		std::cin.ignore(10000, '\n');
		if (response >= 0 && response < 3) {
			choice = response;
		}
		else {
			choice = 0;
		}
		switch (choice) {
		case 1: {
			if (hasWeapon[1] == false && hasWeapon[2] == false && hasWeapon[3] == false) {
				std::cout << "\nYou have no weapons to equip!\n\n";
				inventory();
			}
			else {
				menuEquipWeapon();
			}
			break;
		}
		case 2: {
			if (hasArmor[1] == false && hasArmor[2] == false && hasArmor[3] == false) {
				std::cout << "\nYou have no armor to equip!\n\n";
				inventory();
			}
			else {
				menuEquipArmor();
			}
			break;
		}
		case 0: {
			menuCombat();
			break;
		}

		}

	}
	void entity::menuEquipArmor() {
		int response, choice;
		std::cout << "You have:\n";
		if (hasArmor[1] == true) {
			std::cout << "  1.) A " << armorName[1] << std::endl;
		}
		if (hasArmor[2] == true) {
			std::cout << "  2.) A " << armorName[2] << std::endl;
		}
		if (hasArmor[3] == true) {
			std::cout << "  3.) A " << armorName[3] << std::endl;
		}
		std::cout << "Which of the above would you like to replace" << armorName[armorEquipped] << " with (0 to quit)?";
		std::cin >> response;
		std::cin.clear();
		std::cin.ignore(10000, '\n');
		if (response == armorEquipped) {
			std::cout << "You doff" << armorName[armorEquipped] << "." << std::endl;
			armorEquipped = 0;
			checkEquipment();
			pause();
		}
		if (response >= 0 && response < 4) {
			choice = response;
		}
		else {
			choice = 0;
		}
		switch (choice) {
		case 1: {
			armorEquipped = choice;
			checkEquipment();
			std::cout << "You don" << armorName[armorEquipped] << "!" << std::endl;
			pause();
			break;
		}
		case 2: {
			armorEquipped = choice;
			checkEquipment();
			std::cout << "You don" << armorName[armorEquipped] << "!" << std::endl;
			pause();
			break;
		}
		case 3: {
			armorEquipped = choice;
			checkEquipment();
			std::cout << "You don" << armorName[armorEquipped] << "!" << std::endl;
			pause();
			break;
		}
		case 0: {
			inventory();
			break;

		}
		}
		std::cout << "\n\n\n\n\n\n";
		inventory();
	}
	void entity::menuEquipWeapon() {
		int response, choice;
		std::cout << "You have:\n";
		if (hasWeapon[1] == true) {
			std::cout << "  1.) A " << weaponName[1] << std::endl;
		}
		if (hasWeapon[2] == true) {
			std::cout << "  2.) A " << weaponName[2] << std::endl;
		}
		if (hasWeapon[3] == true) {
			std::cout << "  3.) A " << weaponName[3] << std::endl;
		}
		std::cout << "Which of the above would you like to equip instead of" << weaponName[weaponEquipped] << " (0 to quit)?\n\n";
		std::cin >> response;
		std::cin.clear();
		std::cin.ignore(10000, '\n');
		if (response == 0) {
			std::cout << "You release your grip on" << weaponName[weaponEquipped] << "." << std::endl;
			weaponEquipped = 0;
			checkEquipment();
			pause();
		}
		if (response >= 0 && response < 4) {
			choice = response;
		}
		else {
			choice = 0;
		}
		switch (choice) {
		case 1: {
			weaponEquipped = choice;
			checkEquipment();
			std::cout << "You grab" << weaponName[weaponEquipped] << "!" << std::endl;
			pause();
			break;
		}
		case 2: {
			weaponEquipped = choice;
			checkEquipment();
			std::cout << "You grab" << weaponName[weaponEquipped] << "!" << std::endl;
			pause();
			break;
		}
		case 3: {
			weaponEquipped = choice;
			checkEquipment();
			std::cout << "You grab" << weaponName[weaponEquipped] << "!" << std::endl;
			pause();
			break;
		}
		case 0: {
			inventory();
			break;
		}
			  std::cout << "\n\n\n\n\n\n";
			  inventory();
		}
	}
	int entity::callExperience(int input) { // Function to add XP to the Entity.
		if (input <= 0) {
			return 0;
		}
		else {
			experienceCurrent = experienceCurrent + (input);
			std::cout << "You gain " << input << " experience!\n\n";
			levelCheck();
		}
		return 0;
	}
	void entity::levelUp() { // Function to level the player character up; increases max HP and adds to attack bonus.
		level++;
		std::cout << "\nYou have reached level " << level << "!\n\n";
		attackBase++;
		healthMax = healthMax + (roll(1, 6) + conMod);
	}
	void entity::levelCheck() { // Function to check current XP level for if able to trigger level-up.
		int temp;
		if (experienceCurrent >= experienceToLevel) {
			temp = experienceCurrent - experienceToLevel;
			if (temp < 0) {
				temp = 0;
			}
			levelUp();
			experienceCurrent = temp;
		}
		return;
	}
	void entity::entityRoll() { // Initialization function, creating random stats using the random number simulator below. Does not output any information to client. Also generates static stats to be called automatically.
			// Generation of six base statistics, defined as protected due to lack of need to call specific attributes.
		strength = statRoll();
		dexterity = statRoll();
		constitution = statRoll();
		intelligence = statRoll();
		wisdom = statRoll();
		charisma = statRoll();
		// Applying modifiers to statistics above to create applicable modifiers for rolls. Note that these are protected, so can only be called within the Entity.
		strMod = callMod(strength);
		dexMod = callMod(dexterity);
		conMod = callMod(constitution);
		intMod = callMod(intelligence);
		wisMod = callMod(wisdom);
		chaMod = callMod(charisma);
		// Creating second-removed attributes within the Entity. Note that the base features are protected and, thus, only accessable through the same Entity, preventing tampering or accidental overwrite later.
		armorEquipped = 0;
		weaponEquipped = 0;
		checkEquipment();
		armorBase = 10 + dexMod + itemArmorBonus + itemArmorMagic;
		armorCurrent = armorBase;
		attackBase = dexMod + itemWeaponAttack;
		attackCurrent = attackBase;
		healthMax = 8 + conMod;
		healthCurrent = healthMax;
		damageBase = strMod;
		damageCurrent = damageBase;
		flightCurrent = dexMod;
		initiativeBase = dexMod;
		initiativeCurrent = initiativeBase;
	};
	void entity::introducePlayer() { // Introduction function prints out the details of the character creation for the player to review, as they wish.
	std::cout << "\nDelver " << name << " has:\n" << std::endl;
	std::cout << "\tStrength      " << strength << " (" << strMod << ")" << std::endl;
	std::cout << "\tDexterity     " << dexterity << " (" << dexMod << ")" << std::endl;
	std::cout << "\tConstitution  " << constitution << " (" << conMod << ")" << std::endl;
	std::cout << "\tIntelligence  " << intelligence << " (" << intMod << ")" << std::endl;
	std::cout << "\tWisdom        " << wisdom << " (" << wisMod << ")" << std::endl;
	std::cout << "\tCharisma      " << charisma << " (" << chaMod << ")" << std::endl;
	std::cout << "\nAttack Bonus: " << attackCurrent << std::endl << "Armor Class: " << armorCurrent << std::endl << "HP: " << healthCurrent << " / " << healthMax << std::endl;
	std::cout << "\nYou are fighting with" << weaponName[weaponEquipped] << ".\nYou are wearing" << armorName[armorEquipped] << ".\n" << std::endl;
};
	entity player;
	entity opponent;

int roll(int lowerBound, int upperBound) { // Function for random integer between 1 and (internally-defined) variable.
	int output;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(lowerBound, upperBound);
	output = dis(gen);	
	return output;
};
void pause() { // Function to wait for user input.
	std::cout << "\nPress Enter to continue." << std::endl;
	std::cin.ignore();
};

int main() { // Initial program, leading to the menu.
	mapGen(mapSizeX,mapSizeY);
	menuMain();
	return 0;
}
void menuMain() { // Main menu of the game, first branchpoint reached.
	int response, choice;
	std::cout << "Dragonscar Keep\n" << std::endl << "Choose your fate:" << std::endl << "\t(1) Brave the Keep\n\t(9) Flee in Terror\n" << std::endl;
	std::cin >> response;
	std::cin.clear();
	std::cin.ignore(10000, '\n');
	if (response == 1 || response == 9) {
		choice = response;
	}
	else {
		choice = 9;
	}
	switch (choice) {
		case 1: {
			menuCreation();
			break;
		}
		case 9: {
			exit();
			break;
		}
	}
};
void menuCreation() { // Second "menu", calling functions related to character creation.
	std::cout << "what is your name, adventurer?" << std::endl;
	std::getline(std::cin, player.name);
	creation();
};
	void creation() { // Calling functions for character creation, allowing for the functions to be referred back to as needed.
	player.entityRoll();
	player.introducePlayer();
	checkCreation();
};
	void checkCreation() { // User confirmation required to continue.
	int response, choice;
	std::cout << "Is this acceptable?\n\n";
	std::cout << "\t(1) Enter the keep\n";
	std::cout << "\t(2) Roll again\n\n";
	std::cin >> response;
	std::cin.clear();
	std::cin.ignore(10000, '\n');
	if (response == 1 || response == 2) {
		choice = response;
	}
	else {
		choice = 0;
	}
	switch (choice) {
		case 1: {
			combat();
			break;
		}
		case 2: {
			creation();
			break;
		}
		case 0: {
			std::cout << "Invalid entry. Please try again.\n" << std::endl;
			checkCreation();
			break;
		}
	}
};
void opponentGen() { // Function to generate the "opponent" entity. This will be expanded at some point, to allow for random assignment of entities.
	opponent.name = "Random Monster";
	opponent.entityRoll();
}
void combat() { // Function to call combat.
	int playerInit, opponentInit;
	opponentGen();
	bool combatEnd{ false };
	do {
		std::cout << "\n" << std::endl;
		player.reset();
		opponent.reset();
		menuCombat();
		playerInit = roll(1, 20) + player.initiativeCurrent;
		opponentInit = roll(1, 20) + opponent.initiativeCurrent;
		if (playerInit > opponentInit) {
			
			playerAction();
			if (opponent.deathFlag == true) {
				player.killFlag = true;
				combatEnd = true;
				std::cout << "\n\nYou strike the " << opponent.name << " down!\n";
				combatWin();
				return;
			}
			if (player.flightFlag == true) {
				std::cout << "\n\nYou successfully flee the creature, leaving yourself alone in the dark.\n";
				combatEnd = true;
				combatCleanup();
				pause();
				confirmQuit();
				return;
			}
			opponentAction();
			if (player.deathFlag == true) {
				combatEnd = true;
				std::cout << "\n\nBlackness engulfs you.";
				combatCleanup();
				pause();
				std::cout << "\033[2j" << "\n\n" << player.name << " was killed by the " << opponent.name << "!\n";
				confirmQuit();
				return;
			}
		}
		else {
			opponentAction();
			if (player.deathFlag == true) {
				combatEnd = true;
				std::cout << "\n\nBlackness engulfs you.";
				combatCleanup();
				pause();
				std::cout << "\033[2j" << "\n\n" << player.name << " was killed by the " << opponent.name << "!\n";
				confirmQuit();
				return;
			}
			playerAction();
			if (player.killFlag == true) {
				combatEnd = true;
				std::cout << "\n\nYou strike the " << opponent.name << " down!\n";
				combatCleanup();
				pause();
				confirmQuit();
				return;
			}
			if (player.attack == false && player.rest == false && player.flightFlag == true) {
				std::cout << "\n\nYou successfully flee the creature, leaving yourself alone in the dark.\n";
				combatEnd = true;
				combatCleanup();
				pause();
				confirmQuit();
				return;
			}		
		}
		pause();
	} while (combatEnd == false);
}
	void combatWin() {
		combatCleanup();
		navigation();
		confirmQuit();
}
void menuCombat() { // Function to deliniate character choices from actions, allowing greater diversification.
	int response, choice;
	player.HUD();
	std::cout << "You are fighting a " << opponent.name << ".\n" << std::endl;
	std::cout << "1.) Attack the creature\n" << "2.) Attack the creature without regard\n" << "3.) Attack the creature cautiously\n" << "4.) Catch your breath\n" << "5.) Recover defensively\n" << "\n8.) Change equipment" << "\n\n0.) Attempt to flee\n" << std::endl;
	std::cin >> response;
	std::cin.clear();
	std::cin.ignore(10000, '\n');
	if ( response == 1 || response == 2 || response == 3 || response == 4 || response == 5 || response == 8 || response == 0 ) {
		choice = response;
	}
	else {
		choice = 0;
	}
	switch (choice) {
		case 1: { // Standard attack flags and stat modifiers.
			player.attack = true;
			break;
			}
		case 2: { // Reckless attack flags and stat modifiers.
			player.attack = true;
			player.reckless = true;
			player.attackCurrent = player.attackCurrent + 2;
			player.damageCurrent = player.damageCurrent + 2;
			player.armorCurrent = player.armorCurrent - 4;
			break;
		}
		case 3: { // Cautious attack flaggs and modifiers.
			player.attack = true;
			player.cautious = true;
			player.attackCurrent = player.attackCurrent - 1;
			player.damageCurrent = player.damageCurrent - 1;
			player.armorCurrent = player.armorCurrent + 2;
			break;
		}
		case 4: { // Standard rest flags.
			player.rest = true;
			player.armorCurrent = player.armorCurrent + 1;
			break;
		}
		case 5: { // Reckless rest flags.
			player.rest = true;
			player.reckless = true;
			player.armorCurrent = player.armorCurrent - 2;
			break;
		}
		case 8: {
			player.inventory();
			break;
		}
		case 0: { // Flight flags.
			player.flee = true;
			break;
		}
		default: { // Incorrect response.
			std::cout << "\nIncorrect response!\n" << std::endl;
			menuCombat();
			break;
		}
	
	}
}
	void playerAction() { // Player's action.
	int toHit, toDamage, toHeal, toFlee, toIntercept;
	if (player.attack == true) { // Beginning combat check
		if (player.reckless == true) { // Reckless attack flags
			std::cout << "You swing wildly at the creature, ";
			toHit = player.attackRoll();
			if (toHit >= opponent.armorCurrent) {
				toDamage = player.damageRoll();
				opponent.callDamage(toDamage);
				std::cout << "landing a fierce blow for " << toDamage << " damage!\n" << std::endl;
			}
			else {
				std::cout << "but your frantic swing goes wild, leaving you open!\n" << std::endl;
			}
		}
		if (player.cautious == true) {// Cautious attack flags
			std::cout << "You advance on the creature with caution, ";
			toHit = player.attackRoll();
			if (toHit >= opponent.armorCurrent) {
				toDamage = player.damageRoll();
				opponent.callDamage(toDamage);
				std::cout << " and you deftly strike its moist flesh for " << toDamage << "damage!\n" << std::endl;
			}
			else {
				std::cout << "but your blow does not find purchase on the creature!\n" << std::endl;
			}
		}
		if (player.reckless == false && player.cautious == false) { // Generic attack flags
			std::cout << "You take a practiced swing at the creature, ";
			toHit = player.attackRoll();
			if (toHit >= opponent.armorCurrent) {
				toDamage = player.damageRoll();
				opponent.callDamage(toDamage);
				std::cout << "landing a solid blow for " << toDamage << "damage!\n" << std::endl;
			}
			else {
				std::cout << "but your footing gives way!\n" << std::endl;
			}
		}
	}
	if (player.rest == true) { // Rest-group actions
		if (player.reckless == true) { // Reckless rest action.
			toHeal = (roll(1, 4) + 2);
			player.callHealing(toHeal);
			std::cout << "Dropping your guard, you lean forward to catch your breath, recovering " << toHeal << " health!\n" << std::endl;
		}
		else { // Standard rest action.
toHeal = roll(1, 3);
player.callHealing(toHeal);
std::cout << "You fall back a few steps, while not letting your guard down, and recover " << toHeal << " health!\n" << std::endl;
		}
	}
	if (player.flee == true) { // Flight action
		toFlee = roll(1, 20) + player.flightCurrent;
		toIntercept = roll(1, 20) + opponent.flightCurrent;
		if (toFlee > toIntercept) {
			player.flightFlag = true;
			std::cout << "You turn from the creature and flee into the passage behind you.\n" << std::endl;
		}
		else {
			std::cout << "You turn to flee from the creature, but it surges forward to move between you and your exit!\n" << std::endl;
		}
	}
}
void opponentAction() { // Opponent's action.
	int random, toHit, toDamage;
	random = roll(1, 5);
	switch (random) {
	case 1: { // Heavy attack, trading AC for attack.
		std::cout << "The monster lunges at you, heedless of your attacks, limbs flashing in the dark, and";
		opponent.attackCurrent = opponent.attackCurrent + 2;
		opponent.armorCurrent = opponent.armorCurrent - 4;
		toHit = opponent.attackRoll();
		if (toHit >= player.armorCurrent) {
			toDamage = opponent.damageRoll();
			player.callDamage(toDamage);
			std::cout << " the blow tears at you, dealing " << toDamage << " damage!\n" << std::endl;
		}
		else {
			std::cout << " the blow misses you, the creature's bulk off-balance from the wild swing!\n" << std::endl;
		}
		break;
	}
	case 2: { // Basic attack, with no modifications to rolls
		std::cout << "Grunting in the dark, the creature advances on you with ill intent, ";
		toHit = opponent.attackRoll();
		if (toHit >= player.armorCurrent) {
			toDamage = opponent.damageRoll();
			player.callDamage(toDamage);
			std::cout << "dealing " << toDamage << " damage to you!\n" << std::endl;
		}
		else {
			std::cout << "but you roll past the attack.\n" << std::endl;
		}
		break;
	}
	case 3: { // Cautious attack, trading damage for accuracy.
		std::cout << "Shuffling on the dirt floor they launch a probing limb at you, ";
		opponent.attackCurrent = opponent.attackCurrent + 2;
		opponent.damageCurrent = opponent.damageCurrent - 2;
		toHit = opponent.attackRoll();
		if (toHit >= player.armorCurrent) {
			toDamage = opponent.damageRoll();
			player.callDamage(toDamage);
			std::cout << "sneaking past you defenses to deal " << toDamage << " damage to you!\n" << std::endl;
		}
		else {
			std::cout << "but you manage to deftly dodge the attack.\n" << std::endl;
		}
		break;
	}
	case 4: { // Fortifying themselves for a boost to armor.
		std::cout << "The creature lowers its bulk to the ground, loose stones absorbing into its flesh.\n" << std::endl;
		opponent.armorCurrent = opponent.armorCurrent + 2;
		break;
	}
	case 5: { // Healing themselves for a small sum.
		std::cout << "A deep rumbling emerges from the creature, matching with a wet squelching sound and a grim odor.\n" << std::endl;
		opponent.callHealing(roll(1, 4));
		break;
	}
	};
};
void combatCleanup() { // Fucntion to return all temporary values to their default values after combat.
	player.reset();
	opponent.reset();
	if (player.killFlag == true)
	{
		player.callExperience(200 + (10 * (opponent.attackCurrent + opponent.damageCurrent + opponent.armorCurrent)));
	}
	if (player.flightFlag == true) {
		player.callExperience(10 * (opponent.attackCurrent + opponent.damageCurrent + opponent.initiativeCurrent));
	}
}
void navigation() {
	directionCheck();
	callExits();
	promptNavigation();
	drawMap(mapSizeX, mapSizeY);
}
	void promptNavigation() {
	std::cout << "\n\tWhere would you like to go?";
	char input;
	std::cin >> input;
	if (input == 'N' || input == 'n') {
		transition(1);
	}
	else if (input == 'E' || input == 'e') {
		transition(2);
	}
	else if (input == 'S' || input == 's') {
		transition(3);
	}
	else if (input == 'W' || input == 'w') {
		transition(4);
	}
	else {
		std::cout << "\nIncorrect entry.";
		navigation();
	}
}
	void transition(int direction) {
		switch (direction) {
			case 1: { // Moving character location to the north.
				characterLocationY = characterLocationY - 1;
				break;
			};
			case 2: { // Moving character location to the east.
				characterLocationX = characterLocationX + 1;
				break;
			};
			case 3: { // Moving character location to the south.
				characterLocationY = characterLocationY + 1;
				break;
			};
			case 4: { // Moving character location to the west.
			characterLocationX = characterLocationX - 1;
			break;
		}
		}
		drawRoom();

	}
	void drawRoom() { // Changing the state of the room upon arrival.
		if (map[characterLocationY][characterLocationX] == " ") {
			map[characterLocationY][characterLocationX] = "x";
		}
	}
void mapGen(int xAxis, int yAxis) { // Generating a map of size x*y
	int xCurrent{ 0 }, yCurrent{ 0 };
	drawBorders(xAxis - 1, yAxis - 1);
}
	void drawBorders(int xAxis, int yAxis){ // Drawing borders on the outside of the defined map.
	int xCurrent{ 0 }, yCurrent{ 0 };
	for (xCurrent = 0; xCurrent < xAxis; xCurrent++) {
		map[yCurrent][xCurrent] = "-";
	}
	for (yCurrent = 0; yCurrent < yAxis; yCurrent++) {
		map[yCurrent][xCurrent] = "|";
	}
	for (xCurrent = xAxis; xCurrent > 0; xCurrent--) {
		map[yCurrent][xCurrent] = "-";
	}
	for (yCurrent = yAxis; yCurrent > 0; yCurrent--) {
		map[yCurrent][xCurrent] = "|";
	}
	map[0][0] = map[0][xAxis] = map[yAxis][0] = map[yAxis][xAxis] = "+";
	characterLocationX = (xAxis / 2);
	characterLocationY = (yAxis - 1);
	map[characterLocationY][characterLocationX] = "X";
}
	void drawMap(int xAxis, int yAxis){ // Display map of size x*y;
	int xCurrent{ 0 }, yCurrent{ 0 };
	for (xCurrent = 0; xCurrent <= yAxis; xCurrent++) {
		std::cout << map[yCurrent][xCurrent];
		if (xCurrent == xAxis && yCurrent < yAxis)	{
			std::cout << std::endl;
			yCurrent++;
			xCurrent = -1;
		}
	}
	std::cout << std::endl;
	pause();
}
	void directionCheck() { // Checking the four direcitons to check for available space surrounding the current location.
		exitCount = 0;
		checkNorth();
		checkSouth();
		checkWest();
		checkEast();
	}
		void checkNorth() {
			std::string sample;
			if (map[characterLocationY - 1][characterLocationX] == " ") {
				clearNorth = true;
				exitCount++;
			}
			else clearNorth = false;
			
		}
		void checkEast() {
			std::string sample;
			if (map[characterLocationY][characterLocationX + 1] == " ") {
				clearEast = true;
				exitCount++;
			}
			else clearEast = false;
		}
		void checkWest() {
			std::string sample;
			if (map[characterLocationY][characterLocationX - 1] == " ") {
				clearWest = true;
				exitCount++;
			}
			else clearWest = false;
		}
		void checkSouth() {
			std::string sample;
			if (map[characterLocationY - 1][characterLocationX] == " ") {
				clearSouth = true;
				exitCount++;
			}
			else clearSouth = false;
	}
		void callExits() { // Outputting the available exits, based on the available locations surrounding the current space, as determined by directionCheck().
			if (exitCount == 1) {
				std::cout << "\nYou see an exit to the ";
				if (clearNorth == true) {
					std::cout << "north." << std::endl;
				}
				else if (clearEast == true) {
					std::cout << "east." << std::endl;

				}
				else if (clearSouth == true) {
					std::cout << "south." << std::endl;
				}
				else if (clearWest == true) {
					std::cout << "west." << std::endl;
				}
			}
			else if (exitCount > 1 && exitCount < 5) {
				std::cout << "\nYou see exits to the ";
				if (clearNorth == true) {
					std::cout << "north, ";
				}
				if (clearEast == true) {
					std::cout << "east, ";
				}
				if (clearSouth == true) {
					std::cout << "south, ";
				}
				if (clearWest == true) {
					std::cout << "west, ";
				}
			}
			else {
				std::cout << "no exits from this chamber.";
			}
		}
	void roomCheck() {

	}
void confirmQuit() { // Intended for the end of the adventure, asking if the user would like to try again.
	std::string response;
	std::cout << "\n\n" << "Would you like to take another path? ( Y / N )\n";
	std::cin.clear();
	std::cin.ignore();
	std::getline(std::cin, response);
	if (response == "y") {
		menuCreation();
	};
	if (response == "Y") {
		menuCreation();
	};
	exit();
};
int exit() { // Exit command, leading to the successful termination of the program.
	std::cout << "\n\n\n";
	std::cout << "Not all who wander are lost.";
	return 0;
};
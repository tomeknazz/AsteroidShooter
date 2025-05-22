#include <vector>
#include <algorithm>
#include <functional> 
#include <memory>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include <raylib.h>
#include <raymath.h>

// --- UTILS ---
namespace Utils {
	inline static float RandomFloat(float min, float max) {
		return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
	}
}

// --- TRANSFORM, PHYSICS, LIFETIME, RENDERABLE ---
struct TransformA {
	Vector2 position{};
	float rotation{};
};

struct Physics {
	Vector2 velocity{};
	float rotationSpeed{};
};

struct Renderable {
	enum Size { SMALL = 1, MEDIUM = 2, LARGE = 4 } size = SMALL;
};

// --- RENDERER ---
class Renderer {
public:
	static Renderer& Instance() {
		static Renderer inst;
		return inst;
	}

	void Init(int w, int h, const char* title) {
		InitWindow(w, h, title);
		SetTargetFPS(60);
		screenW = w;
		screenH = h;
		backgroundTexture = LoadTexture("background.png"); 
	}

	void Begin() {
		BeginDrawing();
		DrawTextureEx(
			backgroundTexture,
			{ 0, 0 },
			0.0f,
			std::max(
				(float)Width() / backgroundTexture.width,
				(float)Height() / backgroundTexture.height
			),
			WHITE
		);
	}

	void End() {
		EndDrawing();
	}

	void DrawPoly(const Vector2& pos, int sides, float radius, float rot) {
		DrawPolyLines(pos, sides, radius, rot, WHITE);
	}

	int Width() const {
		return screenW;
	}

	int Height() const {
		return screenH;
	}

private:
	Renderer() = default;

	int screenW{};
	int screenH{};
	Texture2D backgroundTexture;
};

// --- ASTEROID HIERARCHY ---

class Asteroid {
public:
	Asteroid(int screenW, int screenH) {
		init(screenW, screenH);
	}
	virtual ~Asteroid() = default;

	virtual bool Update(float dt) {
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));
		transform.rotation += physics.rotationSpeed * dt;
		if (transform.position.x < -GetRadius() || transform.position.x > Renderer::Instance().Width() + GetRadius() ||
			transform.position.y < -GetRadius() || transform.position.y > Renderer::Instance().Height() + GetRadius())
			return false;
		return true;
	}
	virtual void Draw() const = 0;

	Vector2 GetPosition() const {
		return transform.position;
	}

	float constexpr GetRadius() const {
		return 16.f * (float)render.size;
	}

	int GetDamage() const {
		return baseDamage * static_cast<int>(render.size);
	}

	int GetSize() const {
		return static_cast<int>(render.size);
	}

protected:
	void init(int screenW, int screenH) {
		// Choose size
		render.size = static_cast<Renderable::Size>(1 << GetRandomValue(1, 2));

		// Spawn at random edge
		switch (GetRandomValue(0, 3)) {
		case 0:
			transform.position = { Utils::RandomFloat(0, screenW), -GetRadius() };
			break;
		case 1:
			transform.position = { screenW + GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		case 2:
			transform.position = { Utils::RandomFloat(0, screenW), screenH + GetRadius() };
			break;
		default:
			transform.position = { -GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		}

		// Aim towards center with jitter
		float maxOff = fminf(screenW, screenH) * 0.2f;
		float ang = Utils::RandomFloat(0, 2 * PI);
		float rad = Utils::RandomFloat(0, maxOff);
		Vector2 center = {
										 screenW * 0.5f + cosf(ang) * rad,
										 screenH * 0.5f + sinf(ang) * rad
		};

		Vector2 dir = Vector2Normalize(Vector2Subtract(center, transform.position));
		physics.velocity = Vector2Scale(dir, Utils::RandomFloat(SPEED_MIN, SPEED_MAX));
		physics.rotationSpeed = Utils::RandomFloat(ROT_MIN, ROT_MAX);

		transform.rotation = Utils::RandomFloat(0, 360);
	}

	TransformA transform;
	Physics    physics;
	Renderable render;

	int baseDamage = 0;
	static constexpr float LIFE = 10.f;
	static constexpr float SPEED_MIN = 125.f;
	static constexpr float SPEED_MAX = 250.f;
	static constexpr float ROT_MIN = 50.f;
	static constexpr float ROT_MAX = 240.f;

	Texture2D texture;
};

class TriangleAsteroid : public Asteroid {
public:
	TriangleAsteroid(int w, int h) : Asteroid(w, h) {
		baseDamage = 5;
		texture = LoadTexture("asteroid_triangle.png");
	}
	~TriangleAsteroid() { UnloadTexture(texture); }
	void Draw() const override {
		float scale = GetRadius() * 2.0f / texture.width;
		Vector2 center = transform.position;
		Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
		Rectangle dst = {
			center.x,
			center.y,
			texture.width * scale,
			texture.height * scale
		};
		Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };
		DrawTexturePro(texture, src, dst, origin, transform.rotation, WHITE);
	}
};
class SquareAsteroid : public Asteroid {
public:
	SquareAsteroid(int w, int h) : Asteroid(w, h) {
		baseDamage = 10;
		texture = LoadTexture("asteroid_square.png");
	}
	~SquareAsteroid() { UnloadTexture(texture); }
	void Draw() const override {
		float scale = GetRadius() * 2.0f / texture.width;
		Vector2 center = transform.position;
		Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
		Rectangle dst = {
			center.x,
			center.y,
			texture.width * scale,
			texture.height * scale
		};
		Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };
		DrawTexturePro(texture, src, dst, origin, transform.rotation, WHITE);
	}
};

class PentagonAsteroid : public Asteroid {
public:
	PentagonAsteroid(int w, int h) : Asteroid(w, h) {
		baseDamage = 15;
		texture = LoadTexture("asteroid_pentagon.png");
	}
	~PentagonAsteroid() { UnloadTexture(texture); }
	void Draw() const override {
		float scale = GetRadius() * 2.0f / texture.width;
		Vector2 center = transform.position;
		Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
		Rectangle dst = {
			center.x,
			center.y,
			texture.width * scale,
			texture.height * scale
		};
		Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };
		DrawTexturePro(texture, src, dst, origin, transform.rotation, WHITE);
	}
};

class ChasingAsteroid : public Asteroid {
public:
	ChasingAsteroid(int w, int h, std::function<Vector2()> getPlayerPos, float manualOffsetDeg = 0.0f)
		: Asteroid(w, h), getPlayerPosition(getPlayerPos), manualOffsetRad(manualOffsetDeg* (PI / 180.0f))
	{
		baseDamage = 20;
		texture = LoadTexture("asteroid_chaser.png");
	}
	~ChasingAsteroid() { UnloadTexture(texture); }

	bool Update(float dt) override {
		// Kierunek do gracza
		Vector2 playerPos = getPlayerPosition();
		Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, transform.position));
		float speed = 110.0f;
		physics.velocity = Vector2Scale(dir, speed);

		// Standardowy update pozycji
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));

		// Usuwanie poza ekranem
		if (transform.position.x < -GetRadius() || transform.position.x > Renderer::Instance().Width() + GetRadius() ||
			transform.position.y < -GetRadius() || transform.position.y > Renderer::Instance().Height() + GetRadius())
			return false;
		return true;
	}

	void Draw() const override {
		// kąt do gracza
		Vector2 playerPos = getPlayerPosition();
		float angleToPlayer = atan2f(playerPos.y - transform.position.y, playerPos.x - transform.position.x);

		
		float finalAngle = angleToPlayer + manualOffsetRad;

		float scale = GetRadius() * 2.0f / texture.width;
		Vector2 center = transform.position;
		Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
		Rectangle dst = {
			center.x,
			center.y,
			texture.width * scale,
			texture.height * scale
		};
		Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };
		DrawTexturePro(texture, src, dst, origin, finalAngle * (180.0f / PI), WHITE);
	}

private:
	std::function<Vector2()> getPlayerPosition;
	float manualOffsetRad;
};

// Shape selector
enum class AsteroidShape { TRIANGLE = 3, SQUARE = 4, PENTAGON = 5, RANDOM = 0 };

// Factory
static inline std::unique_ptr<Asteroid> MakeAsteroid(int w, int h, AsteroidShape shape, std::function<Vector2()> getPlayerPos = nullptr) {
	switch (shape) {
	case AsteroidShape::TRIANGLE:
		return std::make_unique<TriangleAsteroid>(w, h);
	case AsteroidShape::SQUARE:
		return std::make_unique<SquareAsteroid>(w, h);
	case AsteroidShape::PENTAGON:
		return std::make_unique<PentagonAsteroid>(w, h);
	case AsteroidShape::RANDOM: {
		
		int r = GetRandomValue(0, 9);
		if (r < 1) // 0,1 -> chasing (20%)
			return std::make_unique<ChasingAsteroid>(w, h, getPlayerPos, 45.0f);
		else {
			int type = GetRandomValue(0, 2);
			switch (type) {
			case 0: return std::make_unique<TriangleAsteroid>(w, h);
			case 1: return std::make_unique<SquareAsteroid>(w, h);
			default: return std::make_unique<PentagonAsteroid>(w, h);
			}
		}
	}
	default:
		return std::make_unique<ChasingAsteroid>(w, h, getPlayerPos, 45.0f);
	}
}

// --- PROJECTILE HIERARCHY ---
enum class WeaponType { LASER, BULLET, COUNT };
class Projectile {
public:
	Projectile(Vector2 pos, Vector2 vel, int dmg, WeaponType wt, Texture2D tex)
		: texture(tex), hasTexture(true)
	{
		transform.position = pos;
		physics.velocity = vel;
		baseDamage = dmg;
		type = wt;
	}

	Projectile(Vector2 pos, Vector2 vel, int dmg, WeaponType wt)
		: hasTexture(false)
	{
		transform.position = pos;
		physics.velocity = vel;
		baseDamage = dmg;
		type = wt;
	}

	bool Update(float dt) {
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));

		if (transform.position.x < 0 ||
			transform.position.x > Renderer::Instance().Width() ||
			transform.position.y < 0 ||
			transform.position.y > Renderer::Instance().Height())
		{
			return true;
		}
		return false;
	}

	void Draw() const {
		if (hasTexture) {
			float scale = 0.2f; // Ustaw skalę według potrzeb
			Vector2 dstPos = {
				transform.position.x - (texture.width * scale) * 0.5f,
				transform.position.y - (texture.height * scale) * 0.5f
			};
			DrawTextureEx(texture, dstPos, 0.0f, scale, WHITE);
		}
		else if (type == WeaponType::BULLET) {
			DrawCircleV(transform.position, 5.f, WHITE);
		}
		else {
			static constexpr float LASER_LENGTH = 30.f;
			Rectangle lr = { transform.position.x - 2.f, transform.position.y - LASER_LENGTH, 4.f, LASER_LENGTH };
			DrawRectangleRec(lr, RED);
		}
	}

	Vector2 GetPosition() const {
		return transform.position;
	}

	float GetRadius() const {
		return (type == WeaponType::BULLET) ? 5.f : 2.f;
	}

	int GetDamage() const {
		return baseDamage;
	}

private:
	TransformA transform;
	Physics    physics;
	int        baseDamage;
	WeaponType type;
	Texture2D  texture;
	bool       hasTexture;
};

inline static Projectile MakeProjectile(WeaponType wt,
	const Vector2 pos,
	float speed)
{
	Vector2 vel{ 0, -speed };
	if (wt == WeaponType::LASER) {
		return Projectile(pos, vel, 20, wt);
	}
	else {
		return Projectile(pos, vel, 10, wt);
	}
}

// --- SHIP HIERARCHY ---
class Ship {
public:
	Ship(int screenW, int screenH) {
		transform.position = {
												 screenW * 0.5f,
												 screenH * 0.5f
		};
		hp = 100;
		speed = 400.f;
		alive = true;

		// per-weapon fire rate & spacing
		fireRateLaser = 18.f; // shots/sec
		fireRateBullet = 22.f;
		spacingLaser = 40.f; // px between lasers
		spacingBullet = 20.f;
	}
	virtual ~Ship() = default;
	virtual void Update(float dt) = 0;
	virtual void Draw() const = 0;

	void TakeDamage(int dmg) {
		if (!alive) return;
		hp -= dmg;
		if (hp <= 0) alive = false;
	}

	bool IsAlive() const {
		return alive;
	}

	Vector2 GetPosition() const {
		return transform.position;
	}

	virtual float GetRadius() const = 0;

	int GetHP() const {
		return hp;
	}

	float GetFireRate(WeaponType wt) const {
		return (wt == WeaponType::LASER) ? fireRateLaser : fireRateBullet;
	}

	float GetSpacing(WeaponType wt) const {
		return (wt == WeaponType::LASER) ? spacingLaser : spacingBullet;
	}

protected:
	TransformA transform;
	int        hp;
	float      speed;
	bool       alive;
	float      fireRateLaser;
	float      fireRateBullet;
	float      spacingLaser;
	float      spacingBullet;
};

class PlayerShip :public Ship {
public:
	// --- OVERHEAT ---
	float overheat = 0.0f;
	float overheatCooldown = 0.0f;
	bool overheated = false;
	static constexpr float OVERHEAT_MAX = 100.0f;
	static constexpr float OVERHEAT_PER_SHOT = 3.0f; // ile za 1 strzał
	static constexpr float OVERHEAT_COOLDOWN_RATE = 30.0f; // ile schodzi na sekundę
	static constexpr float OVERHEAT_COOLDOWN_DELAY = 1.0f; // po strzale, ile czeka zanim zacznie chłodzić
	bool overheatSkillUsed = false;

	PlayerShip(int w, int h) : Ship(w, h) {
		texture = LoadTexture("spaceship2.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2);
		scale = 0.25f;
		bulletTexture = LoadTexture("bullet.png");
	}
	~PlayerShip() {
		UnloadTexture(texture);
		UnloadTexture(bulletTexture);
	}

	void Update(float dt) override {
		if (alive) {
			if (IsKeyDown(KEY_W)) transform.position.y -= speed * dt;
			if (IsKeyDown(KEY_S)) transform.position.y += speed * dt;
			if (IsKeyDown(KEY_A)) transform.position.x -= speed * dt;
			if (IsKeyDown(KEY_D)) transform.position.x += speed * dt;
		}
		else {
			transform.position.y += speed * dt;
		}
		// Chłodzenie broni
		if (overheat > 0.0f) {
			if (overheatCooldown > 0.0f)
				overheatCooldown -= dt;
			else
				overheat = std::max(0.0f, overheat - OVERHEAT_COOLDOWN_RATE * dt);
		}
		if (overheat >= OVERHEAT_MAX) {
			overheated = true;
			overheat = OVERHEAT_MAX;
		}
		if (overheated && overheat <= 0.0f) {
			overheated = false;
			overheatSkillUsed = false;
		}
	}

	void Draw() const override {
		if (!alive && fmodf(GetTime(), 0.4f) > 0.2f) return;
		Vector2 dstPos = {
										 transform.position.x - (texture.width * scale) * 0.5f,
										 transform.position.y - (texture.height * scale) * 0.5f
		};
		DrawTextureEx(texture, dstPos, 0.0f, scale, WHITE);

		// --- HEALTHBAR ---
		// Parametry paska
		float barWidth = 60.0f;
		float barHeight = 8.0f;
		float barOffsetY = (texture.height * scale) * 0.5f + 12.0f; // odległość pod statkiem

		float hpPercent = (float)hp / 100.0f;
		float filledWidth = barWidth * hpPercent;

		Vector2 barPos = {
			transform.position.x - barWidth * 0.5f,
			transform.position.y + barOffsetY
		};

		// Tło paska (szary)
		DrawRectangleV(barPos, { barWidth, barHeight }, DARKGRAY);
		// Wypełnienie (zielony/czerwony)
		Color fillColor = (hpPercent > 0.5f) ? GREEN : (hpPercent > 0.2f ? ORANGE : RED);
		DrawRectangleV(barPos, { filledWidth, barHeight }, fillColor);
		// Ramka
		DrawRectangleLines((int)barPos.x, (int)barPos.y, (int)barWidth, (int)barHeight, BLACK);


		// --- OVERHEAT BAR ---
		float barWidth1 = 16.0f;
		float barHeight1 = 80.0f;
		float barOffsetX1 = (texture.width * scale) * 0.5f + 16.0f;
		float barOffsetY1 = 0.0f;

		Vector2 barPos1 = {
			transform.position.x + barOffsetX1,
			transform.position.y - barHeight1 * 0.5f + barOffsetY1
		};

		// Tło paska (ciemny szary)
		DrawRectangleV(barPos1, { barWidth1, barHeight1 }, DARKGRAY);

		// Wypełnienie (czerwony)
		float fillHeight1 = barHeight1 * GetOverheatPercent();
		Vector2 fillPos1 = { barPos1.x, barPos1.y + barHeight1 - fillHeight1 };
		DrawRectangleV(fillPos1, { barWidth1, fillHeight1 }, (overheated ? ORANGE : RED));

		// Ramka
		DrawRectangleLines((int)barPos1.x, (int)barPos1.y, (int)barWidth1, (int)barHeight1, BLACK);

		// --- OVERHEATED TEXT ---
		if (overheated && fmodf(GetTime(), 0.6f) < 0.3f) {
			const char* txt = "OVERHEATED!";
			int fontSize = 32;
			int textWidth = MeasureText(txt, fontSize);
			Vector2 textPos = {
				transform.position.x - textWidth * 0.5f,
				transform.position.y + (texture.height * scale) * 0.5f + 40.0f // 40px pod statkiem
			};
			DrawText(txt, (int)textPos.x, (int)textPos.y, fontSize, ORANGE);
		}
		// --- PRESS E TEXT ---
		if (overheated && fmodf(GetTime(), 0.8f) < 0.4f && !overheatSkillUsed) {
			const char* txt = "PRESS E";
			int fontSize = 28;
			int textWidth = MeasureText(txt, fontSize);
			Vector2 textPos = {
				transform.position.x - textWidth * 0.5f,
				transform.position.y + (texture.height * scale) * 0.5f + 80.0f // pod napisem OVERHEATED!
			};
			DrawText(txt, (int)textPos.x, (int)textPos.y, fontSize, YELLOW);
		}

	}

	float GetRadius() const override {
		return (texture.width * scale) * 0.5f;
	}

	Texture2D GetBulletTexture() const {
		return bulletTexture;
	}

	bool CanShoot() const { return !overheated; }
	float GetOverheatPercent() const { return overheat / OVERHEAT_MAX; }
	bool IsOverheated() const { return overheated; }

private:
	Texture2D texture;
	float     scale;
	Texture2D bulletTexture;

};

// --- APPLICATION ---
class Application {
public:
	static Application& Instance() {
		static Application inst;
		return inst;
	}

	void Run() {
		srand(static_cast<unsigned>(time(nullptr)));
		Renderer::Instance().Init(C_WIDTH, C_HEIGHT, "Space ship");

		auto player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);

		player->overheat = 0.0f;
		player->overheated = false;
		player->overheatCooldown = 0.0f;

		float spawnTimer = 0.f;
		float spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
		WeaponType currentWeapon = WeaponType::LASER;
		float shotTimer = 0.f;

		score = 0;



		while (!WindowShouldClose()) {
			float dt = GetFrameTime();
			spawnTimer += dt;

			// Update player
			player->Update(dt);

			// Restart logic
			if (!player->IsAlive() && IsKeyPressed(KEY_R)) {
				player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);
				player->overheat = 0.0f;
				player->overheated = false;
				player->overheatCooldown = 0.0f;
				asteroids.clear();
				projectiles.clear();
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
				score = 0; // <-- DODAJ TO
			}
			// Asteroid shape switch
			if (IsKeyPressed(KEY_ONE)) {
				currentShape = AsteroidShape::TRIANGLE;
			}
			if (IsKeyPressed(KEY_TWO)) {
				currentShape = AsteroidShape::SQUARE;
			}
			if (IsKeyPressed(KEY_THREE)) {
				currentShape = AsteroidShape::PENTAGON;
			}
			if (IsKeyPressed(KEY_FOUR)) {
				currentShape = AsteroidShape::RANDOM;
			}
			if (IsKeyPressed(KEY_FIVE)) {
				currentShape = static_cast<AsteroidShape>(6); // Chasing asteroid
			}

			// Weapon switch
			if (IsKeyPressed(KEY_TAB)) {
				currentWeapon = static_cast<WeaponType>((static_cast<int>(currentWeapon) + 1) % static_cast<int>(WeaponType::COUNT));
			}

			// Shooting
			{
				if (player->IsAlive() && IsKeyDown(KEY_SPACE) && player->CanShoot()) {
					shotTimer += dt;
					float interval = 1.f / player->GetFireRate(currentWeapon);
					float projSpeed = player->GetSpacing(currentWeapon) * player->GetFireRate(currentWeapon);

					while (shotTimer >= interval) {
						Vector2 p = player->GetPosition();
						p.y -= player->GetRadius();
						projectiles.push_back(MakeProjectile(currentWeapon, p, projSpeed));
						shotTimer -= interval;

						// --- OVERHEAT ---
						player->overheat += PlayerShip::OVERHEAT_PER_SHOT;
						player->overheatCooldown = PlayerShip::OVERHEAT_COOLDOWN_DELAY;
						if (player->overheat >= PlayerShip::OVERHEAT_MAX) {
							player->overheated = true;
							player->overheat = PlayerShip::OVERHEAT_MAX;
						}
					}
				}
				else {
					float maxInterval = 1.f / player->GetFireRate(currentWeapon);

					if (shotTimer > maxInterval) {
						shotTimer = fmodf(shotTimer, maxInterval);
					}
				}
				if (player->IsAlive() && player->overheated && IsKeyPressed(KEY_E) && !player->overheatSkillUsed) {
					const int numBullets = 50;
					float angleStep = 2 * PI / numBullets;
					float bulletSpeed = 600.0f;
					Vector2 center = player->GetPosition();
					for (int i = 0; i < numBullets; ++i) {
						float angle = i * angleStep;
						Vector2 dir = { cosf(angle), sinf(angle) };
						Vector2 pos = center;
						Vector2 vel = Vector2Scale(dir, bulletSpeed);
						projectiles.push_back(
							Projectile(pos, vel, 10, WeaponType::BULLET, player->GetBulletTexture())
						);
					}
					player->overheatSkillUsed = true;
				}

			}

			// Spawn asteroids
			if (spawnTimer >= spawnInterval && asteroids.size() < MAX_AST) {
				asteroids.push_back(MakeAsteroid(C_WIDTH, C_HEIGHT, currentShape, [&player]() { return player->GetPosition(); }));
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
			}

			// Update projectiles - check if in boundries and move them forward
			{
				auto projectile_to_remove = std::remove_if(projectiles.begin(), projectiles.end(),
					[dt](auto& projectile) {
						return projectile.Update(dt);
					});
				projectiles.erase(projectile_to_remove, projectiles.end());
			}

			// Projectile-Asteroid collisions O(n^2)
			for (auto pit = projectiles.begin(); pit != projectiles.end();) {
				bool removed = false;

				for (auto ait = asteroids.begin(); ait != asteroids.end(); ++ait) {
					float dist = Vector2Distance((*pit).GetPosition(), (*ait)->GetPosition());
					if (dist < (*pit).GetRadius() + (*ait)->GetRadius()) {
						score += 10 * (*ait)->GetSize(); // 10 punktów za SMALL, 20 za MEDIUM, 40 za LARGE
						ait = asteroids.erase(ait);
						pit = projectiles.erase(pit);
						removed = true;
						break;
					}
				}
				if (!removed) {
					++pit;
				}
			}

			// Asteroid-Ship collisions
			{
				auto remove_collision =
					[&player, dt](auto& asteroid_ptr_like) -> bool {
					if (player->IsAlive()) {
						float dist = Vector2Distance(player->GetPosition(), asteroid_ptr_like->GetPosition());

						if (dist < player->GetRadius() + asteroid_ptr_like->GetRadius()) {
							player->TakeDamage(asteroid_ptr_like->GetDamage());
							return true; // Mark asteroid for removal due to collision
						}
					}
					if (!asteroid_ptr_like->Update(dt)) {
						return true;
					}
					return false; // Keep the asteroid
					};
				auto asteroid_to_remove = std::remove_if(asteroids.begin(), asteroids.end(), remove_collision);
				asteroids.erase(asteroid_to_remove, asteroids.end());
			}

			// Render everything
			{
				Renderer::Instance().Begin();

				DrawText(TextFormat("HP: %d", player->GetHP()),
					10, 10, 48, GREEN); // większy rozmiar czcionki

				const char* weaponName = (currentWeapon == WeaponType::LASER) ? "LASER" : "BULLET";
				DrawText(TextFormat("Weapon: %s", weaponName),
					10, 70, 48, BLUE); // większy rozmiar czcionki i przesunięcie w dół

				DrawText(TextFormat("Score: %d", score),
					10, 130, 48, YELLOW); // pozycja pod HP i Weapon, rozmiar i kolor możesz zmienić

				DrawText(TextFormat("Overheat: %.1f", player->overheat),
					10, 190, 48, RED); // wyświetlanie poziomu przegrzania

				for (const auto& projPtr : projectiles) {
					projPtr.Draw();
				}
				for (const auto& astPtr : asteroids) {
					astPtr->Draw();
				}

				player->Draw();

				Renderer::Instance().End();
			}
		}
	}

private:
	Application()
	{
		asteroids.reserve(1000);
		projectiles.reserve(10'000);
		score = 0;
	};
	int score;
	std::vector<std::unique_ptr<Asteroid>> asteroids;
	std::vector<Projectile> projectiles;

	AsteroidShape currentShape = AsteroidShape::RANDOM;

	Texture2D backgroundTexture;

	static constexpr int C_WIDTH = 2560;
	static constexpr int C_HEIGHT = 1400;
	static constexpr size_t MAX_AST = 150;
	static constexpr float C_SPAWN_MIN = 0.15f;
	static constexpr float C_SPAWN_MAX = 0.5f;

	static constexpr int C_MAX_ASTEROIDS = 1000;
	static constexpr int C_MAX_PROJECTILES = 10'000;
};

int main() {
	Application::Instance().Run();
	return 0;
}
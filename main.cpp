#include <Gorgon/Geometry/Point.h>
#include <Gorgon/Geometry/PointList.h>
#include <Gorgon/Geometry/Size.h>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Color.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/Main.h>
#include <Gorgon/UI.h>
#include <Gorgon/Scene.h>
#include <Gorgon/Game/Renderer/Tiled/Renderer.h>
#include <Gorgon/Game/World/World.h>

inline int zoom = 2;

class Game {
    using GridScene = Gorgon::Game::Rendering::Tiled::StandardRenderer; 

    static Gorgon::Geometry::Point pixel_to_tile(Gorgon::Geometry::Point p) {
        return {(p.X + 1) / (16 * zoom), (p.Y + 1) / (16 * zoom)};
    }

    public: 
    explicit Game(Gorgon::SceneManager& w_) : world(w_) {
        world.NewScene(0, std::move(*new GridScene{{"map.tmx"}}), true);
        world.SwitchScene(0); 
        world.ExecuteForActiveScene<GridScene>([&](Gorgon::Game::Scene<GridScene>& scene){
            scene.GetRenderer().Unprepare();
            scene.GetRenderer().PrepareZoomed(zoom); 
            scene.SetBackgroundRender(false); 

            player.image.Import("character.png"); 
            player.image = player.image.ZoomMultiple(zoom); 
            player.image.Prepare(); 

            scene.GetPathFinder().SetSize({16 * zoom, 16 * zoom}); 
            for(auto tile : scene.GetRenderer().GetActiveMap().GetPassabilityLayer().data_to_grid()) {
                if(not tile.is_passable()) {
                    scene.GetPathFinder().AddBlock(tile.location); 
                }
            }

            scene.OnMouseMove.Register([&](Gorgon::Geometry::Point loc) {
                mouse_location = loc; 
            });

            scene.OnRender.Register([&](Gorgon::Graphics::Layer& graphics) {
                scene.GetRenderer().Render(); 
                player.image.Draw(graphics, player.location); 
                scene.GetRenderer().BoundsOnPoint(mouse_location, 2., Gorgon::Graphics::Color::Cyan); 
            }); 

            scene.OnMouseDown.Register([&](Gorgon::Geometry::Point location) {
                auto temp = scene.GetPathFinder().FindPath(pixel_to_tile(player.location), pixel_to_tile(location));
                if(*temp.begin() != pixel_to_tile(location)) {
                    return; 
                }
                for(auto& node : temp) {
                    node.X = int(node.X * scene.GetRenderer().GetActiveMap().tilewidth);
                    node.Y = int(node.Y * scene.GetRenderer().GetActiveMap().tileheight);
                }
                path = temp.Duplicate();
            });

            scene.OnUpdate.Register([&](unsigned  int delta) {
                if(path.IsEmpty()) { return; }
                player.target = path.Back();

                Gorgon::Geometry::Pointf vector = (player.target - player.location);

                if(vector.Distance() > 1) {
                    vector = vector.Normalize() * player.speed * delta / 1000;
                    player.location += vector;
                } else {
                    path.Erase(path.end() - 1);
                }
            });

        }); 
    }

    private:
    struct {
        int speed = 200; 
        Gorgon::Graphics::Bitmap image; 
        Gorgon::Geometry::Point location = {16 * zoom, 16 * zoom}, target = {16 ,16}; 
    } player; 

    Gorgon::Geometry::Point mouse_location; 
    Gorgon::Geometry::PointList<> path; 
    Gorgon::Game::World<Gorgon::Game::EmptyInitializer> world; 
}; 
int main() {
    Gorgon::Initialize("SimpleGame Camera");
    Gorgon::UI::Initialize();

    Gorgon::SceneManager window({16 * 30 * zoom, 16 * 30 * zoom}, "Simple Game Camera", "Simple Game Camera"); 
    Game game{window};
    window.Run(); 
    
    return 0;
}
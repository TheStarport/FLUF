## Advanced Navigation Map

This plugin requires FLUF.UI. It provides an advanced navigation map system for Freelancer that allows players to:

### Features

- **System Overview**: View all discovered systems on a navigable grid
- **Detailed System Maps**: Zoom into individual systems to see planets, bases, and other points of interest
- **Player Position Tracking**: See your current position in both system overview and detailed views
- **Discovery System**: Automatically tracks visited systems and locations
- **Interactive Navigation**: Click on systems to explore them in detail
- **Coordinate Display**: View precise coordinates for navigation planning

### Usage

- Press the USER_STATUS key (default: F10) to toggle the navigation map
- Click on systems in the overview to enter detailed system view
- Right-click or use the Back button to return to system overview
- Click anywhere in a system map to select coordinates

### Configuration

The module can be configured via `modules/config/advanced_navmap.yml`:

```yaml
displayable_systems:
  - li01
  - li02
  - br01
  # Add more systems as needed

show_player_position: true
show_system_grid: true
show_coordinates: true
map_scale: 1.0
```

### Dependencies

- FLUF.UI (for ImGui interface)
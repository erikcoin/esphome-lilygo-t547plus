This repository contains a Display component for [ESPHome](https://esphome.io/)
to support the ESP32-S3 [M5PaperS3 ESP32S3](https://shop.m5stack.com/products/m5papers3-esp32s3-development-kit?srsltid=AfmBOop9YrPU-bZLrfjE2Xw-rBs4XxZaDxewgWWLK_s-THUg6zBpm0h6).

For more info in the display components, see the [ESPHome display documentation](https://esphome.io/#display-components)

## Usage

To use the board with [ESPHome](https://esphome.io/) **you have to put quite a
number of options in your esphome config**:
* Configure the aprpopriate board, variant, and framework versions in the
[esp32 platform](https://esphome.io/components/esp32.html)
* Set a bunch of `platformio_options`
* Include the component from this repository as `external_components` 

If you clone this repository, a working example is included:

    git clone https://github.com/erikcoin/esphome-lilygo-t547plus.git
    cd esphome-lilygo-t547plus
    esphome run basic.yaml

If you don't want to clone, copy the necessary pieces from [basic.yaml](./basic.yaml)
and adapt the `external_components` configuration as follows:

```yaml
# ... required esp32, platformio_options configuration omitted for brevity ...

external_components:
  - source: github://erikcoin/esphome-lilygo-t547plus
    components: ["papers33"]


display:
  - platform: papers33
    id: m5paper_display
    rotation: 0
    update_interval: 30s # How often the main display content redraws
    touch_coordinates: touch_coordinates # Link to the text_sensor
    # Define your buttons here
    buttons:
      - x_grid: 50
        y_grid: 100
        width: 200
        buttonid: knop1
        height: 80
        on_press:
          #- script.execute: test_button_action

## Discussion

https://github.com/esphome/feature-requests/issues/1960

#include "colors.inc"
#include "textures.inc"

// unit is meter

#declare _tex = texture {
  pigment { color White }
}

#declare RndSeed = seed(30);
#declare _posCamera = <1.0,0.5,1.0>;
#declare _lookAt = <0.0,0.0,0.0>;

camera {
  location    _posCamera
  look_at     _lookAt
  right x
}

light_source {
  <1.0, 1.0, 0.5>
  color rgb 1.5
  area_light <-0.1, 0, -0.1>, <0.1, 0, 0.1>, 3, 3
  adaptive 1
  jitter
}

background { color rgb <0.9,0.9,1.0> }

global_settings { ambient_light 0 }

#declare hotelIcon = union {
  #local groundSize = 0.75;
  #local buildingWidth = 0.5;
  #local buildingHeight = 1.5;
  // Ground
  box {
    <-groundSize,-0.1,-groundSize>
    <groundSize,0.0,groundSize>
    texture {
      pigment {
        color <0.75, 1.0, 0.75>
      }
    }
  }
  // Flowers
  box {
    <groundSize-0.05,0.15,0.1>
    <groundSize-0.2,0.05,0.4>
    texture {
      pigment {
        color <0.5, 1.0, 0.5>
      }
    }
  }
  box {
    <groundSize-0.05,0.15,-0.1>
    <groundSize-0.2,0.05,-0.4>
    texture {
      pigment {
        color <0.5, 1.0, 0.5>
      }
    }
  }
  sphere {
    <groundSize-0.1,0.175,0.35>
    0.04
    texture {
      pigment {
        color <0.5, 0.5, 1.0>
      }
    }
  }
  sphere {
    <groundSize-0.15,0.175,0.15>
    0.04
    texture {
      pigment {
        color <1.0, 0.5, 0.5>
      }
    }
  }
  sphere {
    <groundSize-0.05,0.175,-0.15>
    0.04
    texture {
      pigment {
        color <0.5, 0.5, 1.0>
      }
    }
  }
  sphere {
    <groundSize-0.25,0.175,-0.35>
    0.04
    texture {
      pigment {
        color <1.0, 0.5, 1.0>
      }
    }
  }
  // Board
  box {
    <-buildingWidth,buildingHeight*0.4,buildingWidth-0.02>
    <-buildingWidth-0.2,buildingHeight*0.9,buildingWidth-0.01>
    texture {
      pigment {
        color <1.0,1.0,1.0>
      }
      finish {
        ambient .9
      }
    }
  }  
  #local colBuilding = <0.85, 0.85, 0.95>;
  // Building
  difference {
    // Main block
    box {
      <-buildingWidth,0.0,-buildingWidth>
      <buildingWidth,buildingHeight,buildingWidth>
    }
    union {
      // Roof terrasse
      box {
        <-buildingWidth*0.95,buildingHeight-0.05,-buildingWidth*0.95>
        <buildingWidth*0.95,buildingHeight+1.0,buildingWidth*0.95>
      }
      // Entrance door
      union {
        box {
          <-0.075,0.1,0.0>
          <0.075,0.25,buildingWidth+0.1>
        }
        cylinder {
          <0.0,0.25,0.0>
          <0.0,0.25,buildingWidth+0.1>
          0.075
        }      
      }
      // Floor windows
      #local iFloor=0;
      #local nbFloor=4;
      #while (iFloor<nbFloor)
        #local iWindow=1;
        #local nbWindow=5;
        #while (iWindow<nbWindow)
          box {
            <-buildingWidth+2.0*buildingWidth*(iWindow/nbWindow)+0.05,(iFloor+1.3)*(buildingHeight/(nbFloor+1)),-1.0>
            <-buildingWidth+2.0*buildingWidth*(iWindow/nbWindow)-0.05,(iFloor+1.8)*(buildingHeight/(nbFloor+1)),1.0>
          }
          box {
            <-buildingWidth+2.0*buildingWidth*(iWindow/nbWindow)+0.05,(iFloor+1.3)*(buildingHeight/(nbFloor+1)),-1.0>
            <-buildingWidth+2.0*buildingWidth*(iWindow/nbWindow)-0.05,(iFloor+1.8)*(buildingHeight/(nbFloor+1)),1.0>
            rotate 90.0*y
          }
          #declare iWindow=iWindow+1;
        #end
        #declare iFloor=iFloor+1;
      #end
    }
    texture {
      pigment {
        color colBuilding
      }
    }
  }
  // Glass
  box {
    <-buildingWidth,buildingHeight*0.2,buildingWidth-0.05>
    <buildingWidth,buildingHeight,buildingWidth-0.01>
    texture {
      Glass
    }
  }
  // Entrance stairways
  union {
    box {
      <-0.2,0.0,0.0>
      <0.2,0.05,buildingWidth+0.15>
    }
    box {
      <-0.15,0.05,0.0>
      <0.15,0.1,buildingWidth+0.1>
    }
    texture {
      pigment {
        color colBuilding
      }
    }
  }
  // Terrasse block
  box {
    <0.0,buildingHeight-1.0,0.0>
    <0.25,buildingHeight+0.1,-0.25>
    texture {
      pigment {
        color colBuilding
      }
    }
  }
}

object {
  hotelIcon
  translate -0.5*y
  scale 0.55
  rotate 20.0 * y
}


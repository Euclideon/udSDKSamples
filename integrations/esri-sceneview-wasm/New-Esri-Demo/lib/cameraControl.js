const locations = [
  {
    name: "Melbourne", 
    description: "Melbourne City", 
    camera: {
      position: {
        x: 16134291.687088,
        y: -4555398.704025,
        z: 575.375238,
        spatialReference: { 
          wkid: 102100
        },
      },
      heading: 43.7650864,
      tilt: 64.840576
    }
  },
  {
    name: "Bendigo", 
    description: "Bendigo City", 
    camera: {
      position: {
        x: 16061786.289466,
        y: -4406420.012302,
        z: 339.154327,
        spatialReference: { 
          wkid: 102100
        },
      },
      heading: 320.817253,
      tilt: 70.099063
    }
  }
];

/**
  @description Fly the camera without calling camera.viewer.flyto - we do this in here.
  @param {Cesium.viewer} viewer requires cesium viewer object
  @param {String} place requires a string to determine where to fly to
  @param {Number} duration requires a number to determine how long to should the flight takes.
*/
const flyCamera = (view, place, duration) => {
  if(view == null) {
    console.log("View object is not defined!")
    return;
  }

  locations.map((location) => {
    if(place === location.name) {
      // If the place is matched in the location array, we fly to camera.
      const options = {
        speedFactor: 0.33, // animation is 3 times slower than default
        easing: "out-quint" // easing function to slow down when reaching the target
      };
    
      view.goTo(location.camera, options);
      return true;
    }
  });
}

const defaultViewpoint = () => {
  return ({
    camera: {
      position: {
        x: 14798566.57611913,
        y: -3147250.482783899,
        z: 12059425.86571629,
        spatialReference: { 
          wkid: 102100
        },
      },
      heading: 3.78,
      tilt: 0.25
    }
  });
}

const moveCameraToAus = (view) => {
  const options = {
    speedFactor: 0.2, // animation is 5 times slower than default
    easing: "out-quint" // easing function to slow down when reaching the target
  };

  view.goTo({
    position: {
      x: 14798566.57611913,
      y: -3147250.482783899,
      z: 12059425.86571629,
      spatialReference: { 
        wkid: 102100
      },
    },
    heading: 3.78,
    tilt: 0.25
  }, options);
}
import React, { Component } from "react";
import * as THREE from "three";
import {OrbitControls} from "three/examples/jsm/controls/OrbitControls";

const style = {
    height: 500// we can control scene size by setting container dimensions
};

class Pitch3D extends Component {
    componentDidMount() {
        this.sceneSetup();
        this.addCustomSceneObjects();
        this.startAnimationLoop();
        window.addEventListener("resize", this.handleWindowResize);
    }

    componentWillUnmount() {
        window.removeEventListener("resize", this.handleWindowResize);
        window.cancelAnimationFrame(this.requestID);
        this.controls.dispose();
    }

    // Standard scene setup in Three.js. Check "Creating a scene" manual for more information
    // https://threejs.org/docs/#manual/en/introduction/Creating-a-scene
    sceneSetup = () => {
        // get container dimensions and use them for scene sizing
        const width = this.el.clientWidth;
        const height = this.el.clientHeight;

        this.scene = new THREE.Scene();
        this.camera = new THREE.PerspectiveCamera(
            50, // fov = field of view
            width / height, // aspect ratio
            0.1, // near plane
            1000 // far plane
        );
        this.camera.position.z = 5;
        this.controls = new OrbitControls(this.camera, this.el);
        this.renderer = new THREE.WebGLRenderer();
        this.renderer.setSize(width, height);
        this.el.appendChild(this.renderer.domElement); // mount using React ref
    };

    addCustomSceneObjects = () => {
        const geometry = new THREE.SphereGeometry( 0.12, 32, 32 );
        const material = new THREE.MeshBasicMaterial( {color: 0x34b7eb} );
        const sphere = new THREE.Mesh( geometry, material );
        sphere.position.set(0.63, 10.92, 37.38)
        this.scene.add( sphere );

        const size = 10;
        const divisions = 10;
        const gridHelper = new THREE.GridHelper( size, divisions );
        this.scene.add( gridHelper );

        //Strike Zone
        const line_material = new THREE.LineBasicMaterial( { color: 0xFFFFFF, linewidth: 2, linecap: 'round'} );
        const points = [];
        points.push(new THREE.Vector3(0.83, 3.67, 1.4));
        points.push(new THREE.Vector3(-0.83, 3.67, 1.4));
        points.push(new THREE.Vector3(-0.83, 1.52, 1.4));
        points.push(new THREE.Vector3(0.83, 1.52, 1.4));
        points.push(new THREE.Vector3(0.83, 3.67, 1.4));
        const line_geometry = new THREE.BufferGeometry().setFromPoints( points );
        const line = new THREE.Line(line_geometry, line_material);
        this.scene.add(line);



        //Home plate
        let tmpPts = [
            new THREE.Vector3(-0.7, 0, 1.4),
            new THREE.Vector3(0.7, 0, 1.4),
            new THREE.Vector3(0.7, 0,0.7),
            new THREE.Vector3(0, 0, 0),
            new THREE.Vector3(-0.7, 0, 0.7),
        ];
        let homePts = tmpPts.map(p=>{return new THREE.Vector2(p.x, -p.z)});
        let homePlate = new THREE.Shape(homePts);
        let plateGeom = new THREE.ShapeBufferGeometry(homePlate);
        plateGeom.rotateX(-Math.PI * 0.5);
        let plateMat = new THREE.MeshBasicMaterial({color: "white"});
        let plateMesh = new THREE.Mesh(plateGeom, plateMat);
        this.scene.add(plateMesh);


        console.log(this.props.trajectory);
        //Trajectory of the ball
        let curve = new THREE.CatmullRomCurve3(this.props.trajectory.map(p=>{
            return new THREE.Vector3(p[0], p[2], p[1]);
        }));
        const trajecPoints = curve.getPoints( 50 );
        const trajecGeometry = new THREE.BufferGeometry().setFromPoints( trajecPoints );
        const trajecMaterial = new THREE.LineBasicMaterial( { color : 0xff0000, linewidth: 10 } );
        // Create the final object to add to the scene
        const curveObject = new THREE.Line( trajecGeometry,trajecMaterial);
        this.scene.add(curveObject)




        const axesHelper = new THREE.AxesHelper( 30);
        this.scene.add( axesHelper );
        const lights = [];
        lights[0] = new THREE.PointLight(0xffffff, 1, 0);
        lights[1] = new THREE.PointLight(0xffffff, 1, 0);
        lights[2] = new THREE.PointLight(0xffffff, 1, 0);

        lights[0].position.set(0, 200, 0);
        lights[1].position.set(100, 200, 100);
        lights[2].position.set(-100, -200, -100);

        this.scene.add(lights[0]);
        this.scene.add(lights[1]);
        this.scene.add(lights[2]);
    };

    startAnimationLoop = () => {

        this.renderer.render(this.scene, this.camera);

        // The window.requestAnimationFrame() method tells the browser that you wish to perform
        // an animation and requests that the browser call a specified function
        // to update an animation before the next repaint
        this.requestID = window.requestAnimationFrame(this.startAnimationLoop);
    };

    handleWindowResize = () => {
        const width = this.el.clientWidth;
        const height = this.el.clientHeight;

        this.renderer.setSize(width, height);
        this.camera.aspect = width / height;

        // Note that after making changes to most of camera properties you have to call
        // .updateProjectionMatrix for the changes to take effect.
        this.camera.updateProjectionMatrix();
    };

    render() {
        return <div style={style} ref={ref => (this.el = ref)} />;
    }
}


class Container extends React.Component {
    state = { isMounted: true };

    render() {
        const { isMounted = true } = this.state;
        return (
            <>
                <button
                    onClick={() =>
                        this.setState(state => ({ isMounted: !state.isMounted }))
                    }
                >
                    {isMounted ? "Hide Animation" : "Show Animation"}
                </button>
                {isMounted && <Pitch3D trajectory={this.props.trajectory}/>}
                {isMounted && <div>Scroll to zoom, drag to rotate</div>}
            </>
        );
    }
}



export default Container;

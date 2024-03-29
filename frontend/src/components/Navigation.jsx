import React from "react";
import { Link, withRouter } from "react-router-dom";

function Navigation(props) {
    return (
        <div className="navigation">
            <nav class="navbar navbar-expand navbar-dark bg-dark">
                <div class="container">
                    <Link class="navbar-brand" to="/">
                        Baseball Simulator
                    </Link>

                    <div>
                        <ul class="navbar-nav ml-auto">
                            <li
                                class={`nav-item  ${
                                    props.location.pathname === "/" ? "active" : ""
                                }`}
                            >
                                <Link class="nav-link" to="/">
                                    Home
                                </Link>
                            </li>
                            <li
                                class={`nav-item  ${
                                    props.location.pathname === "/Dev" ? "active" : ""
                                }`}
                            >
                                <Link class="nav-link" to="/Dev">
                                   Dev
                                </Link>
                            </li>
                            <li
                                class={`nav-item  ${
                                    props.location.pathname === "/contact" ? "active" : ""
                                }`}
                            >
                                <Link class="nav-link" to="/contact">
                                    Contact
                                </Link>
                            </li>
                        </ul>
                    </div>
                </div>
            </nav>
        </div>
    );
}

export default withRouter(Navigation);
import { useEffect, useState } from "react";
import { Col, Container, Row } from "react-bootstrap"
import Person from "./Person";

const WallOfShame = (props) => {

    const [criminals, setCriminals] = useState([]);

    useEffect(() => {
        fetch('https://coletnelson.us/cs839/api/criminals')
            .then(res => res.json())
            .then(data => {
                setCriminals(data.criminals.sort(c => c.datetaken));
            })
    }, [])

    return <div>
        <h1>BAC Trac</h1>
        <div style={{ alignContent: "left", textAlign: "left" }}>
            <p>There are several reasons why drinking alcohol in a laboratory setting is generally discouraged...</p>
            <ol>
                <li><strong>Safety:</strong> Alcohol impairs judgment and reaction time, which could lead to accidents and potentially harm both the individual and others in the laboratory. This is especially important in laboratory settings where hazardous programming languages or libraries are used.</li>
                <li><strong>Professionalism:</strong> Drinking alcohol in the workplace is generally not considered professional behavior and could harm the reputation of both the individual and the organization.</li>
                <li><strong>Interference with Work:</strong> Alcohol can impact an individual's ability to perform their job effectively, which could negatively impact the accuracy and quality of their work in the laboratory.</li>
                <li><strong>Legal Implications:</strong> Depending on the laws and regulations of the jurisdiction, drinking alcohol in a workplace may be illegal. Additionally, if an individual is found to be under the influence of alcohol in the workplace, they could face disciplinary action.</li>
            </ol>
            <p>In general, it is best to avoid consuming alcohol in a laboratory setting and to prioritize safety, professionalism, and compliance with regulations.</p>
            <sub>Text generated using <a href="https://chat.openai.com/chat" target="_blank">ChatGPT</a>. Interested in building a website like this? <a href="https://cs571.org/" target="_blank">Take CS571</a>.</sub>
        </div>
        <h2>Wall of Shame</h2>
        <Container>
            <Row>
                {
                    criminals.map(c => <Col xs={12} md={6} lg={4}><Person {...c} /></Col>)
                }
            </Row>
        </Container>
    </div>
}

export default WallOfShame;
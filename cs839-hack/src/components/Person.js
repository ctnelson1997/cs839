import { Button, Image } from "react-bootstrap";

const Person = (props) => {

    const dateTaken = new Date(props.datetaken);

    return <div>
        <h2>WANTED!</h2>
        <Image src={props.img} width={300} height={300}></Image>
        <p>Caught on {dateTaken.toLocaleDateString()} at {dateTaken.toLocaleTimeString()}</p>
        <p>Recognize this person?</p>
        <Button>Call the Authorities</Button>
    </div>
}

export default Person;